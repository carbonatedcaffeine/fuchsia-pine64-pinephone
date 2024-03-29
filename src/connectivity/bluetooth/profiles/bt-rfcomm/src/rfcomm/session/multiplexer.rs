// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use {
    fuchsia_bluetooth::types::Channel,
    futures::channel::mpsc,
    log::{trace, warn},
    std::collections::HashMap,
};

use crate::rfcomm::{
    channel::SessionChannel,
    frame::{Frame, UserData},
    types::{RfcommError, Role, DLCI, MAX_RFCOMM_FRAME_SIZE},
};

/// The parameters associated with this Session.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct SessionParameters {
    /// Whether credit-based flow control is being used for this session.
    pub credit_based_flow: bool,

    /// The max MTU size of a frame.
    pub max_frame_size: usize,
}

impl SessionParameters {
    /// Combines the current session parameters with the `other` parameters and returns
    /// a negotiated SessionParameters.
    fn negotiated(&self, other: &SessionParameters) -> Self {
        // Our implementation is OK with credit based flow. We choose whatever the new
        // configuration requests.
        let credit_based_flow = other.credit_based_flow;
        // Use the smaller (i.e more restrictive) max frame size.
        let max_frame_size = std::cmp::min(self.max_frame_size, other.max_frame_size);
        Self { credit_based_flow, max_frame_size }
    }

    /// Returns true if credit-based flow control is set.
    pub fn credit_based_flow(&self) -> bool {
        self.credit_based_flow
    }

    pub fn max_frame_size(&self) -> usize {
        self.max_frame_size
    }
}

impl Default for SessionParameters {
    fn default() -> Self {
        // Credit based flow must always be preferred - see RFCOMM 5.5.3.
        Self { credit_based_flow: true, max_frame_size: MAX_RFCOMM_FRAME_SIZE }
    }
}

/// The current state of the session parameters.
#[derive(Clone, Copy, PartialEq)]
pub enum ParameterNegotiationState {
    /// Parameters have not been negotiated.
    NotNegotiated,
    /// Parameters are currently being negotiated.
    Negotiating,
    /// Parameters have been negotiated.
    Negotiated(SessionParameters),
}

impl ParameterNegotiationState {
    /// Returns the current parameters.
    ///
    /// If the parameters have not been negotiated, then the default is returned.
    fn parameters(&self) -> SessionParameters {
        match self {
            Self::Negotiated(params) => *params,
            Self::Negotiating | Self::NotNegotiated => SessionParameters::default(),
        }
    }

    /// Returns true if the parameters have been negotiated.
    fn is_negotiated(&self) -> bool {
        match self {
            Self::Negotiated(_) => true,
            Self::Negotiating | Self::NotNegotiated => false,
        }
    }

    /// Negotiates the `new` parameters with the (potentially) current parameters. Returns
    /// the parameters that were set.
    fn negotiate(&mut self, new: SessionParameters) -> SessionParameters {
        let updated = self.parameters().negotiated(&new);
        *self = Self::Negotiated(updated);
        updated
    }
}

/// The `SessionMultiplexer` manages channels over the range of valid User-DLCIs. It is responsible
/// for maintaining the current state of the RFCOMM Session, and provides an API to create,
/// establish, and relay user data over the multiplexed channels.
///
/// The `SessionMultiplexer` is considered "started" when its Role has been assigned.
/// The parameters for the multiplexer must be negotiated before the first DLCI has
/// been established. RFCOMM 5.5.3 states that renegotiation of parameters is optional - this
/// multiplexer will simply echo the current parameters in the event a negotiation request is
/// received after the first DLC is opened and established.
pub struct SessionMultiplexer {
    /// The role for the multiplexer.
    role: Role,

    /// The parameters for the multiplexer.
    parameters: ParameterNegotiationState,

    /// Local opened RFCOMM channels for this session.
    channels: HashMap<DLCI, SessionChannel>,
}

impl SessionMultiplexer {
    pub fn create() -> Self {
        Self {
            role: Role::Unassigned,
            parameters: ParameterNegotiationState::NotNegotiated,
            channels: HashMap::new(),
        }
    }

    pub fn role(&self) -> Role {
        self.role
    }

    pub fn parameter_negotiation_state(&self) -> ParameterNegotiationState {
        self.parameters
    }

    pub fn set_role(&mut self, role: Role) {
        self.role = role;
    }

    /// Returns true if credit-based flow control is enabled.
    pub fn credit_based_flow(&self) -> bool {
        self.parameters().credit_based_flow()
    }

    /// Returns true if the session parameters have been negotiated.
    pub fn parameters_negotiated(&self) -> bool {
        self.parameters.is_negotiated()
    }

    /// Returns the parameters associated with this session.
    pub fn parameters(&self) -> SessionParameters {
        self.parameters.parameters()
    }

    pub fn set_parameters_negotiating(&mut self) {
        self.parameters = ParameterNegotiationState::Negotiating;
    }

    /// Negotiates the parameters associated with this session - returns the session parameters
    /// that were set.
    pub fn negotiate_parameters(
        &mut self,
        new_session_parameters: SessionParameters,
    ) -> SessionParameters {
        // The session parameters can only be modified if no DLCs have been established.
        if self.dlc_established() {
            warn!(
                "Received negotiation request when at least one DLC has already been established"
            );
            return self.parameters();
        }

        // Otherwise, it is OK to negotiate the multiplexer parameters.
        let updated = self.parameters.negotiate(new_session_parameters);
        trace!("Updated Session parameters: {:?}", updated);
        updated
    }

    /// Returns true if the multiplexer has started.
    pub fn started(&self) -> bool {
        self.role.is_multiplexer_started()
    }

    /// Starts the session multiplexer and assumes the provided `role`. Returns Ok(()) if mux
    /// startup is successful.
    pub fn start(&mut self, role: Role) -> Result<(), RfcommError> {
        // Re-starting the multiplexer is not valid, as this would invalidate any opened
        // RFCOMM channels.
        if self.started() {
            return Err(RfcommError::MultiplexerAlreadyStarted);
        }

        // Role must be a valid started role.
        if !role.is_multiplexer_started() {
            return Err(RfcommError::InvalidRole(role));
        }

        self.set_role(role);
        trace!("Session multiplexer started with role: {:?}", role);
        Ok(())
    }

    /// Returns true if the provided `dlci` has been initialized and established in
    /// the multiplexer.
    pub fn dlci_established(&self, dlci: &DLCI) -> bool {
        self.channels.get(dlci).map(|c| c.is_established()).unwrap_or(false)
    }

    /// Returns true if at least one DLC has been established.
    pub fn dlc_established(&self) -> bool {
        self.channels
            .iter()
            .fold(false, |acc, (_, session_channel)| acc | session_channel.is_established())
    }

    /// Finds or initializes a new SessionChannel for the provided `dlci`. Returns a mutable
    /// reference to the channel.
    pub fn find_or_create_session_channel(&mut self, dlci: DLCI) -> &mut SessionChannel {
        let channel = self.channels.entry(dlci).or_insert(SessionChannel::new(dlci, self.role));
        channel
    }

    /// Attempts to establish a SessionChannel for the provided `dlci`.
    /// `user_data_sender` is used by the SessionChannel to relay any received UserData
    /// frames from the client associated with the channel.
    ///
    /// Returns the remote end of the channel on success.
    pub fn establish_session_channel(
        &mut self,
        dlci: DLCI,
        user_data_sender: mpsc::Sender<Frame>,
    ) -> Result<Channel, RfcommError> {
        // If the session parameters have not been negotiated, set them to the default.
        if !self.parameters_negotiated() {
            self.negotiate_parameters(SessionParameters::default());
        }

        // Potentially reserve a new SessionChannel for the provided DLCI.
        let channel = self.find_or_create_session_channel(dlci);
        if channel.is_established() {
            return Err(RfcommError::ChannelAlreadyEstablished(dlci));
        }

        // Create endpoints for the multiplexed channel. Establish the local end and
        // return the remote end.
        let (local, remote) = Channel::create();
        channel.establish(local, user_data_sender);
        Ok(remote)
    }

    /// Closes the SessionChannel for the provided `dlci`. Returns true if the SessionChannel
    /// was closed.
    pub fn close_session_channel(&mut self, dlci: &DLCI) -> bool {
        self.channels.remove(dlci).is_some()
    }

    /// Sends `user_data` to the SessionChannel associated with the `dlci`.
    pub fn send_user_data(&mut self, dlci: DLCI, user_data: UserData) -> Result<(), RfcommError> {
        if let Some(session_channel) = self.channels.get_mut(&dlci) {
            return session_channel.receive_user_data(user_data);
        }
        Err(RfcommError::InvalidDLCI(dlci))
    }
}

// TODO(fxbug.dev/61923): IWBN to have focused tests for the `SessionMultiplexer`. Currently, it's
// transitively tested by the tests for `SessionInner`.
