// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Watchers handles a list of watcher connections attached to a directory.  Watchers as described
//! in io.fidl.

use {
    failure::Fail,
    fidl_fuchsia_io::{
        WatchedEvent, MAX_FILENAME, WATCH_EVENT_EXISTING, WATCH_EVENT_IDLE, WATCH_MASK_EXISTING,
        WATCH_MASK_IDLE,
    },
    fuchsia_async::Channel,
    fuchsia_zircon::MessageBuf,
    futures::{task::Waker, Poll},
    std::iter,
};

/// Type of the errors that might be generated by the [`Watchers::add`] method.
#[derive(Debug, Fail)]
pub enum WatchersAddError {
    /// Provided name of an entry was longer than MAX_FILENAME bytes.  Watcher connection will be
    /// dropped.
    #[fail(display = "Provided entry name exceeds MAX_FILENAME bytes")]
    NameTooLong,
    /// FIDL communication error.  Failure occured while trying to send initial events to the newly
    /// connected watcher.  Watcher connection will be dropped.
    #[fail(display = "Error sending initial list of entries due to a FIDL error")]
    FIDL(fidl::Error),
}

/// Type of the errors that might be generated when trying to send new event to watchers.
#[derive(Debug, Fail)]
pub enum WatchersSendError {
    /// Provided name of an entry was longer than MAX_FILENAME bytes.
    #[fail(display = "Provided entry name exceeds MAX_FILENAME bytes")]
    NameTooLong,
}

/// Wraps all watcher connections observing one directory.  The directory is responsible for
/// calling [`send_event`], [`remove_dead`] and [`add`] methods when appropriate to make sure
/// watchers are observing a consistent view.
pub struct Watchers {
    connections: Vec<WatcherConnection>,
}

impl Watchers {
    /// Constructs a new Watchers instance with no connected watchers.
    pub fn new() -> Self {
        Watchers { connections: Vec::new() }
    }

    /// Connects a new watcher (connected over the `channel`) to the list of watchers.  This
    /// watcher will receive WATCH_EVENT_EXISTING event with all the names provided by the `names`
    /// argument.  `mask` is the event mask this watcher has requested.
    // NOTE Initially I have used `&mut Iterator<Item = &str>` as a type for `names`.  But that
    // effectively says that names of all the entries should exist for the lifetime of the
    // iterator.  As one may store those references for the duration of the lifetime of the
    // iterator reference.
    //
    // When entry names are dynamically generated, as in the "lazy" directory this could be
    // inefficient.  I would not want the iterator to own all the entry names.  At the same time,
    // `add()` does not really need for those names to exist all the time, it only processes one
    // name at a time and never returns to the previous name.
    //
    // One way to be precise about the actual usage pattern that `add` is following is to use a
    // "sink" instead of an iterator, controlled by the producer.  `names` then becomes:
    //
    //   names: FnMut(&mut FnMut(&str) -> bool),
    //
    // `add()` will call `names()` providing a "sink" that will see entry names one at time.  Sink
    // may stop the iteration if it returns `false`.  Calling `names` again will continue the
    // iteration.  But, this interface is quite unusual, as most people will probably expect to see
    // an `Iterator` here.
    //
    // Switching to `AsRef<str>` allows `add` to accept both `Iterator<Item = &str>`, for the case
    // when all the entry names exist for the lifetime of the iterator, as well as `Iterator<Item =
    // String>` for the case, when ownership of the entry names need to be passed on to the `add()`
    // method itself.
    pub fn add<Name>(
        &mut self,
        names: &mut Iterator<Item = Name>,
        mask: u32,
        channel: Channel,
    ) -> Result<(), WatchersAddError>
    where
        Name: AsRef<str>,
    {
        let conn = WatcherConnection::new(mask, channel);

        conn.send_events_existing(names)?;
        conn.send_event_idle()?;

        self.connections.push(conn);
        Ok(())
    }

    /// Informs all the connected watchers about the specified event.  While `mask` and `event`
    /// carry the same information, as they are represented by `WATCH_MASK_*` and `WATCH_EVENT_*`
    /// constants in io.fidl, it is easier when both forms are provided.  `mask` is used to filter
    /// out those watchers that did not request for observation of this event and `event` is used
    /// to construct the event object.  The method will operate correctly only if `mask` and
    /// `event` match.
    ///
    /// In case of a communication error with any of the watchers, connection to this watcher is
    /// closed.
    pub fn send_event<Name>(
        &mut self,
        mask: u32,
        event: u8,
        name: Name,
    ) -> Result<(), WatchersSendError>
    where
        Name: AsRef<str>,
    {
        let name = name.as_ref();

        if name.len() >= MAX_FILENAME as usize {
            return Err(WatchersSendError::NameTooLong);
        }

        self.connections.retain(|watcher| match watcher.send_event_check_mask(mask, event, name) {
            Ok(()) => true,
            Err(ConnectionSendError::NameTooLong) => {
                // This assertion is never expected to trigger as we checked the name length above.
                // It should indicate some kind of bug in the send_event_check_mask() logic.
                panic!(
                    "send_event_check_mask() returned NameTooLong.\n\
                     Max length in bytes: {}\n\
                     Name length: '{}'\n\
                     Name: '{}'",
                    MAX_FILENAME,
                    name.len(),
                    name
                );
            }
            Err(ConnectionSendError::FIDL(_)) => false,
        });

        Ok(())
    }

    /// Informs all the connected watchers about the specified event.  While `mask` and `event`
    /// carry the same information, as they are represented by `WATCH_MASK_*` and `WATCH_EVENT_*`
    /// constants in io.fidl, it is easier when both forms are provided.  `mask` is used to filter
    /// out those watchers that did not request for observation of this event and `event` is used
    /// to construct the event object.  The method will operate correctly only if `mask` and
    /// `event` match.
    ///
    /// In case of a communication error with any of the watchers, connection to this watcher is
    /// closed.
    #[allow(unused)]
    pub fn send_events<GetNames, Names, Name>(
        &mut self,
        mask: u32,
        event: u8,
        mut get_names: GetNames,
    ) -> Result<(), WatchersSendError>
    where
        GetNames: FnMut() -> Names,
        Names: Iterator<Item = Name>,
        Name: AsRef<str>,
    {
        let mut res = Ok(());

        self.connections.retain(|watcher| {
            let mut names = get_names();
            match watcher.send_events_check_mask(mask, event, &mut names) {
                Ok(()) => true,
                Err(ConnectionSendError::NameTooLong) => {
                    // This is not a connection failure, so we are still keeping the connection
                    // alive.
                    res = Err(WatchersSendError::NameTooLong);
                    true
                }
                Err(ConnectionSendError::FIDL(_)) => false,
            }
        });

        res
    }

    /// Checks if there are any connections in the list of watcher connections that has been
    /// closed.  Removes them and setup up the waker to wake up when any of the live connections is
    /// closed.
    pub fn remove_dead(&mut self, waker: &Waker) {
        self.connections.retain(|watcher| !watcher.is_dead(waker));
    }

    /// Returns true if there are any active watcher connections.
    pub fn has_connections(&self) -> bool {
        self.connections.len() != 0
    }

    /// Closes all the currently connected watcher connections.  New connections may still be added
    /// via add().
    #[allow(unused)]
    pub fn close_all(&mut self) {
        self.connections.clear();
    }
}

struct WatcherConnection {
    mask: u32,
    channel: Channel,
}

/// Type of the errors that may occure when trying to send a message over one connection.
enum ConnectionSendError {
    /// Provided name was longer than MAX_FILENAME bytes.
    NameTooLong,
    /// FIDL communication error.
    FIDL(fidl::Error),
}

impl From<ConnectionSendError> for WatchersAddError {
    fn from(err: ConnectionSendError) -> WatchersAddError {
        match err {
            ConnectionSendError::NameTooLong => WatchersAddError::NameTooLong,
            ConnectionSendError::FIDL(err) => WatchersAddError::FIDL(err),
        }
    }
}

impl From<fidl::Error> for ConnectionSendError {
    fn from(err: fidl::Error) -> ConnectionSendError {
        ConnectionSendError::FIDL(err)
    }
}

impl WatcherConnection {
    fn new(mask: u32, channel: Channel) -> Self {
        WatcherConnection { mask, channel }
    }

    /// A helper used by other send_event*() methods.  Sends a collection of
    /// fidl_fuchsia_io::WatchEvent instances over this watcher connection.  Will check to make
    /// sure that `name` fields in the [`WatchedEvent`] instances do not exceed [`MAX_FILENAME`].
    /// Will skip those entries where `name` exceeds the [`MAX_FILENAME`] bytes and will return
    /// [`ConnectionSendError::NameTooLong`] error in that case.
    /// `len` field should be `0`, it will be set to be equal to the length of the `name` field.
    fn send_event_structs<Events>(&self, events: Events) -> Result<(), ConnectionSendError>
    where
        Events: Iterator<Item = WatchedEvent>,
    {
        // Unfortunately, io.fidl currently does not provide encoding for the watcher events.
        // Seems to be due to
        //
        //     https://fuchsia.atlassian.net/browse/ZX-2645
        //
        // As soon as that is fixed we should switch to the generated binding.
        //
        // For now this code duplicates what the C++ version is doing:
        //
        //     https://fuchsia.googlesource.com/zircon/+/1dcb46aa1c4001e9d1d68b8ff5d8fae0c00fbb49/system/ulib/fs/watcher.cpp
        //
        // There is no Transaction wrapping the messages, as for the full blown FIDL events.

        let mut res = Ok(());

        let buffer = &mut vec![];
        let (bytes, handles) = (&mut vec![], &mut vec![]);
        for mut event in events {
            // Make an attempt to send as many names as possible, skipping those that exceed the
            // limit.
            if event.name.len() >= MAX_FILENAME as usize {
                if res.is_ok() {
                    res = Err(ConnectionSendError::NameTooLong);
                }
                continue;
            }

            // `len` has to match the length of the `name` field.  There is no reason to allow the
            // caller to specify anything but 0 here.  We make sure `name` is bounded above, and
            // now we can actually calculate correct value for `len`.
            assert_eq!(event.len, 0);
            event.len = event.name.len() as u8;

            // Keep bytes and handles across loop iterations, to reduce reallocations.
            bytes.clear();
            handles.clear();
            fidl::encoding::Encoder::encode(bytes, handles, &mut event)
                .map_err(ConnectionSendError::FIDL)?;
            if handles.len() > 0 {
                panic!("WatchedEvent struct is not expected to contain any handles")
            }

            if buffer.len() + bytes.len() >= fidl_fuchsia_io::MAX_BUF as usize {
                self.channel
                    .write(&*buffer, &mut vec![])
                    .map_err(fidl::Error::ServerResponseWrite)?;

                buffer.clear();
            }

            buffer.append(bytes);
        }

        if buffer.len() > 0 {
            self.channel.write(&*buffer, &mut vec![]).map_err(fidl::Error::ServerResponseWrite)?;
        }

        res
    }

    /// Constructs and sends a fidl_fuchsia_io::WatchEvent instance over the watcher connection.
    ///
    /// `event` is one of the WATCH_EVENT_* constants, with the values used to populate the `event`
    /// field.
    fn send_event<Name>(&self, event: u8, name: Name) -> Result<(), ConnectionSendError>
    where
        Name: AsRef<str>,
    {
        self.send_event_structs(&mut iter::once(WatchedEvent {
            event,
            len: 0,
            name: name.as_ref().as_bytes().to_vec(),
        }))
    }

    /// Constructs and sends a fidl_fuchsia_io::WatchEvent instance over the watcher connection.
    ///
    /// `event` is one of the WATCH_EVENT_* constants, with the values used to populate the `event`
    /// field.
    #[allow(unused)]
    fn send_events<Name>(
        &self,
        event: u8,
        names: &mut Iterator<Item = Name>,
    ) -> Result<(), ConnectionSendError>
    where
        Name: AsRef<str>,
    {
        self.send_event_structs(&mut names.map(|name| WatchedEvent {
            event,
            len: 0,
            name: name.as_ref().as_bytes().to_vec(),
        }))
    }

    /// Constructs and sends a fidl_fuchsia_io::WatchEvent instance over the watcher connection,
    /// skipping the operation if the watcher did not request this kind of events to be delivered -
    /// filtered by the mask value.
    #[allow(unused)]
    fn send_event_check_mask<Name>(
        &self,
        mask: u32,
        event: u8,
        name: Name,
    ) -> Result<(), ConnectionSendError>
    where
        Name: AsRef<str>,
    {
        if self.mask & mask == 0 {
            return Ok(());
        }

        self.send_event(event, name)
    }

    /// Constructs and sends multiple fidl_fuchsia_io::WatchEvent instances over the watcher
    /// connection, skipping the operation if the watcher did not request this kind of events to be
    /// delivered - filtered by the mask value.
    fn send_events_check_mask<Name>(
        &self,
        mask: u32,
        event: u8,
        names: &mut Iterator<Item = Name>,
    ) -> Result<(), ConnectionSendError>
    where
        Name: AsRef<str>,
    {
        if self.mask & mask == 0 {
            return Ok(());
        }

        self.send_events(event, names)
    }

    /// Sends one fidl_fuchsia_io::WatchEvent instance of type WATCH_EVENT_EXISTING, for every name
    /// in the list.  If the watcher has requested this kind of events - similar to to
    /// [`send_event_check_mask`] above, but with a predefined mask and event type.
    fn send_events_existing<Name>(
        &self,
        names: &mut Iterator<Item = Name>,
    ) -> Result<(), ConnectionSendError>
    where
        Name: AsRef<str>,
    {
        if self.mask & WATCH_MASK_EXISTING == 0 {
            return Ok(());
        }

        self.send_event_structs(&mut names.map(|name| WatchedEvent {
            event: WATCH_EVENT_EXISTING,
            len: 0,
            name: name.as_ref().as_bytes().to_vec(),
        }))
    }

    /// Sends one instance of fidl_fuchsia_io::WatchEvent of type WATCH_MASK_IDLE.  If the watcher
    /// has requested this kind of events - similar to to [`send_event_check_mask`] above, but with
    /// the predefined mask and event type.
    fn send_event_idle(&self) -> Result<(), ConnectionSendError> {
        if self.mask & WATCH_MASK_IDLE == 0 {
            return Ok(());
        }

        self.send_event(WATCH_EVENT_IDLE, "")
    }

    /// Checks if the watcher has closed the connection.  And sets the waker to trigger when the
    /// connection is closed if it was still opened during the call.
    fn is_dead(&self, lw: &Waker) -> bool {
        let channel = &self.channel;

        if channel.is_closed() {
            return true;
        }

        // Make sure we will be notified when the watcher has closed its connected or when any
        // message is send.
        //
        // We are going to close the connection when we receive any message as this is currently an
        // error.  When we fix ZX-2645 and wrap the watcher connection with FIDL, it would be up to
        // the binding code to fail on any unexpected messages.  At that point we can switch to
        // fuchsia_async::OnSignals and only monitor for the close event.
        //
        // We rely on [`Channel::recv_from()`] to invoke [`Channel::poll_read()`], which would call
        // [`RWHandle::poll_read()`] that would set the signal mask to `READABLE | CLOSE`.
        let mut msg = MessageBuf::new();
        match channel.recv_from(&mut msg, lw) {
            // We are not expecting any messages.  Returning true would cause this watcher
            // connection to be dropped and closed as a result.
            Poll::Ready(_) => true,
            // Poll::Pending is actually the only value we are expecting to see from a watcher that
            // did not close it's side of the connection.  And when the connection is closed, we
            // are expecting Poll::Ready(Err(Status::PEER_CLOSED.into_raw())), but that is covered
            // by the case above.
            Poll::Pending => false,
        }
    }
}
