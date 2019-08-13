// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! The omaha_client::common module contains those types that are common to many parts of the
//! library.  Many of these don't belong to a specific sub-module.

use crate::{
    protocol::{self, request::InstallSource, Cohort},
    state_machine::update_check::AppResponse,
    storage::Storage,
};
use futures::lock::Mutex;
use itertools::Itertools;
use log::error;
use serde_derive::{Deserialize, Serialize};
use std::fmt;
use std::rc::Rc;
use std::str::FromStr;
use std::time::SystemTime;

/// Omaha has historically supported multiple methods of counting devices.  Currently, the
/// only recommended method is the Client Regulated - Date method.
///
/// See https://github.com/google/omaha/blob/master/doc/ServerProtocolV3.md#client-regulated-counting-date-based
#[derive(Clone, Debug, Deserialize, Eq, PartialEq, Serialize)]
pub enum UserCounting {
    ClientRegulatedByDate(
        /// Date (sent by the server) of the last contact with Omaha.
        Option<i32>,
    ),
}

/// Helper implementation to bridge from the protocol to the internal representation for tracking
/// the data for client-regulated user counting.
impl From<Option<protocol::response::DayStart>> for UserCounting {
    fn from(opt_day_start: Option<protocol::response::DayStart>) -> Self {
        match opt_day_start {
            Some(day_start) => UserCounting::ClientRegulatedByDate(day_start.elapsed_days),
            None => UserCounting::ClientRegulatedByDate(None),
        }
    }
}

/// Omaha only supports versions in the form of A.B.C.D, A.B.C, A.B or A.  This is a utility
/// wrapper around that form of version.
#[derive(Clone, Eq, Ord, PartialEq, PartialOrd)]
pub struct Version(pub Vec<u32>);

impl fmt::Display for Version {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0.iter().format("."))
    }
}

impl fmt::Debug for Version {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        // The Debug trait just forwards to the Display trait implementation for this type
        fmt::Display::fmt(self, f)
    }
}

#[derive(Debug, failure::Fail)]
struct TooManyNumbersError;

impl fmt::Display for TooManyNumbersError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        f.write_str("Too many numbers in version, the maximum is 4.")
    }
}

impl FromStr for Version {
    type Err = failure::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let nums = s.split('.').map(|s| s.parse::<u32>()).collect::<Result<Vec<u32>, _>>()?;
        if nums.len() > 4 {
            return Err(TooManyNumbersError.into());
        }
        Ok(Version(nums))
    }
}

impl From<Vec<u32>> for Version {
    fn from(v: Vec<u32>) -> Self {
        Version(v)
    }
}

macro_rules! impl_from {
    ($($t:ty),+) => {
        $(
            impl From<$t> for Version {
                fn from(v: $t) -> Self {
                    Version(v.to_vec())
                }
            }
        )+
    }
}
impl_from!(&[u32], [u32; 1], [u32; 2], [u32; 3], [u32; 4]);

/// The App struct holds information about an application to perform an update check for.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct App {
    /// This is the app_id that Omaha uses to identify a given application.
    pub id: String,

    /// This is the current version of the application.
    pub version: Version,

    /// This is the fingerprint for the application package.
    ///
    /// See https://github.com/google/omaha/blob/master/doc/ServerProtocolV3.md#packages--fingerprints
    pub fingerprint: Option<String>,

    /// The app's current cohort information (cohort id, hint, etc).  This is both provided to Omaha
    /// as well as returned by Omaha.
    pub cohort: Cohort,

    /// The app's current user-counting information.  This is both provided to Omaha as well as
    /// returned by Omaha.
    pub user_counting: UserCounting,
}

/// Structure used to serialize per app data to be persisted.
/// Be careful when making changes to this struct to keep backward compatibility.
#[derive(Clone, Debug, Deserialize, Eq, PartialEq, Serialize)]
pub struct PersistedApp {
    pub cohort: Cohort,
    pub user_counting: UserCounting,
}

impl From<&App> for PersistedApp {
    fn from(app: &App) -> Self {
        PersistedApp { cohort: app.cohort.clone(), user_counting: app.user_counting.clone() }
    }
}

impl App {
    /// Construct an App from an ID and version, from anything that can be converted into a String
    /// and a Version.
    pub fn new<I: Into<String>, V: Into<Version>>(id: I, version: V, cohort: Cohort) -> Self {
        App {
            id: id.into(),
            version: version.into(),
            fingerprint: None,
            cohort,
            user_counting: UserCounting::ClientRegulatedByDate(None),
        }
    }

    /// Construct an App from an ID, version, and fingerprint.  From anything that can be converted
    /// into a String and a Version.
    pub fn with_fingerprint<I: Into<String>, V: Into<Version>, F: Into<String>>(
        id: I,
        version: V,
        fingerprint: F,
        cohort: Cohort,
    ) -> Self {
        App {
            id: id.into(),
            version: version.into(),
            fingerprint: Some(fingerprint.into()),
            cohort,
            user_counting: UserCounting::ClientRegulatedByDate(None),
        }
    }

    /// Load data from |storage|, only overwrite existing fields if data exists.
    pub async fn load<'a>(&'a mut self, storage: &'a impl Storage) {
        if let Some(app_json) = storage.get_string(&self.id).await {
            match serde_json::from_str::<PersistedApp>(&app_json) {
                Ok(persisted_app) => {
                    // Do not overwrite existing fields in app if no data for this field is
                    // persisted.
                    if let Some(id) = persisted_app.cohort.id {
                        self.cohort.id = Some(id);
                    }
                    if let Some(hint) = persisted_app.cohort.hint {
                        self.cohort.hint = Some(hint);
                    }
                    if let Some(name) = persisted_app.cohort.name {
                        self.cohort.name = Some(name);
                    }
                    if let UserCounting::ClientRegulatedByDate(Some(days)) =
                        persisted_app.user_counting
                    {
                        self.user_counting = UserCounting::ClientRegulatedByDate(Some(days));
                    }
                }
                Err(e) => {
                    error!("Unable to deserialize PersistedApp from json {}: {}", app_json, e);
                }
            }
        }
    }

    /// Persist cohort and user counting to |storage|, will try to set all of them to storage even
    /// if previous set fails.
    /// It will NOT call commit() on |storage|, caller is responsible to call commit().
    pub async fn persist<'a>(&'a self, storage: &'a mut impl Storage) {
        let persisted_app = PersistedApp::from(self);
        match serde_json::to_string(&persisted_app) {
            Ok(json) => {
                if let Err(e) = storage.set_string(&self.id, &json).await {
                    error!("Unable to persist cohort id: {}", e);
                }
            }
            Err(e) => {
                error!("Unable to serialize PersistedApp {:?}: {}", persisted_app, e);
            }
        }
    }

    /// Get the current channel name from cohort name, returns default stable-channel if no cohort
    /// name set for the app.
    pub fn get_current_channel(&self) -> &str {
        self.cohort.name.as_ref().map(|s| s.as_str()).unwrap_or("stable-channel")
    }

    /// Get the target channel name from cohort hint, fallback to current channel if no hint.
    pub fn get_target_channel(&self) -> &str {
        self.cohort.hint.as_ref().map(|s| s.as_str()).unwrap_or(self.get_current_channel())
    }

    /// Set the cohort hint to |channel|.
    pub fn set_target_channel(&mut self, channel: Option<String>) {
        self.cohort.hint = channel;
    }
}

/// A set of Apps.
#[derive(Clone, Debug)]
pub struct AppSet {
    apps: Rc<Mutex<Vec<App>>>,
}

impl AppSet {
    /// Create a new AppSet with the given `apps`.
    ///
    /// # Panics
    ///
    /// Panics if `apps` is empty.
    pub fn new(apps: Vec<App>) -> Self {
        assert!(!apps.is_empty());
        AppSet { apps: Rc::new(Mutex::new(apps)) }
    }

    /// Load data from |storage|, only overwrite existing fields if data exists.
    pub async fn load<'a>(&'a mut self, storage: &'a impl Storage) {
        let mut apps = self.apps.lock().await;
        for app in apps.iter_mut() {
            app.load(storage).await;
        }
    }

    /// Persist cohort and user counting to |storage|, will try to set all of them to storage even
    /// if previous set fails.
    /// It will NOT call commit() on |storage|, caller is responsible to call commit().
    pub async fn persist<'a>(&'a self, storage: &'a mut impl Storage) {
        let apps = self.apps.lock().await;
        for app in apps.iter() {
            app.persist(storage).await;
        }
    }

    /// Get the current channel name from cohort name, returns default stable-channel if no cohort
    /// name set for the app.
    pub async fn get_current_channel(&self) -> String {
        let apps = self.apps.lock().await;
        apps[0].get_current_channel().to_string()
    }

    /// Get the target channel name from cohort hint, fallback to current channel if no hint.
    pub async fn get_target_channel(&self) -> String {
        let apps = self.apps.lock().await;
        apps[0].get_target_channel().to_string()
    }

    /// Set the cohort hint of all apps to |channel|.
    pub async fn set_target_channel(&self, channel: Option<String>) {
        let mut apps = self.apps.lock().await;
        for app in apps.iter_mut() {
            app.set_target_channel(channel.clone());
        }
    }

    /// Update the cohort for each app from Omaha app response.
    pub async fn update_from_omaha(&mut self, app_responses: Vec<AppResponse>) {
        let mut apps = self.apps.lock().await;

        for app_response in app_responses {
            for app in apps.iter_mut() {
                if app.id == app_response.app_id {
                    app.cohort.update_from_omaha(app_response.cohort);
                    break;
                }
            }
        }
    }

    /// Clone the apps into a Vec.
    pub async fn to_vec(&self) -> Vec<App> {
        self.apps.lock().await.clone()
    }
}

/// Options controlling a single update check
#[derive(Clone, Debug, Default)]
pub struct CheckOptions {
    /// Was this check initiated by a person that's waiting for an answer?
    ///  This is used to ignore the background poll rate, and to be aggressive about
    ///  failing fast, so as not to hang on not receiving a response.
    pub source: InstallSource,
}

/// This describes the data around the scheduling of update checks
#[derive(Clone, Debug, PartialEq)]
pub struct UpdateCheckSchedule {
    /// When the last update check was attempted (start time of the check process).
    pub last_update_time: SystemTime,

    /// When the next periodic update window starts.
    pub next_update_window_start: SystemTime,

    /// When the update should happen (in the update window).
    pub next_update_time: SystemTime,
}

#[cfg(test)]
impl Default for UpdateCheckSchedule {
    fn default() -> Self {
        UpdateCheckSchedule {
            last_update_time: SystemTime::UNIX_EPOCH,
            next_update_time: SystemTime::UNIX_EPOCH,
            next_update_window_start: SystemTime::UNIX_EPOCH,
        }
    }
}

/// These hold the data maintained request-to-request so that the requirements for
/// backoffs, throttling, proxy use, etc. can all be properly maintained.  This is
/// NOT the state machine's internal state.
#[derive(Clone, Debug, Default)]
pub struct ProtocolState {
    /// If the server has dictated the next poll interval, this holds what that
    /// interval is.
    pub server_dictated_poll_interval: Option<std::time::Duration>,

    /// The number of consecutive failed update attempts.
    pub consecutive_failed_update_attempts: u32,

    /// The number of consecutive failed update checks.  Used to perform backoffs.
    pub consecutive_failed_update_checks: u32,

    /// The number of consecutive proxied requests.  Used to periodically not use
    /// proxies, in the case of an invalid proxy configuration.
    pub consecutive_proxied_requests: u32,
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{state_machine::update_check::Action, storage::MemStorage};
    use futures::executor::block_on;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_version_display() {
        let version = Version::from([1, 2, 3, 4]);
        assert_eq!("1.2.3.4", version.to_string());

        let version = Version::from([0, 6, 4, 7]);
        assert_eq!("0.6.4.7", version.to_string());
    }

    #[test]
    fn test_version_debug() {
        let version = Version::from([1, 2, 3, 4]);
        assert_eq!("1.2.3.4", format!("{:?}", version));

        let version = Version::from([0, 6, 4, 7]);
        assert_eq!("0.6.4.7", format!("{:?}", version));
    }

    #[test]
    fn test_version_parse() {
        let version = Version::from([1, 2, 3, 4]);
        assert_eq!("1.2.3.4".parse::<Version>().unwrap(), version);

        let version = Version::from([6, 4, 7]);
        assert_eq!("6.4.7".parse::<Version>().unwrap(), version);

        let version = Version::from([999]);
        assert_eq!("999".parse::<Version>().unwrap(), version);
    }

    #[test]
    fn test_version_parse_leading_zeros() {
        let version = Version::from([1, 2, 3, 4]);
        assert_eq!("1.02.003.0004".parse::<Version>().unwrap(), version);

        let version = Version::from([6, 4, 7]);
        assert_eq!("06.4.07".parse::<Version>().unwrap(), version);

        let version = Version::from([999]);
        assert_eq!("0000999".parse::<Version>().unwrap(), version);
    }

    #[test]
    fn test_version_parse_error() {
        assert!("1.2.3.4.5".parse::<Version>().is_err());
        assert!("1.2.".parse::<Version>().is_err());
        assert!(".1.2".parse::<Version>().is_err());
        assert!("-1".parse::<Version>().is_err());
        assert!("abc".parse::<Version>().is_err());
        assert!(".".parse::<Version>().is_err());
        assert!("".parse::<Version>().is_err());
        assert!("999999999999999999999999".parse::<Version>().is_err());
    }

    #[test]
    fn test_version_compare() {
        assert!(Version::from([1, 2, 3, 4]) < Version::from([2, 0, 3]));
        assert!(Version::from([1, 2, 3]) < Version::from([1, 2, 3, 4]));
    }

    #[test]
    fn test_app_new_version() {
        let app = App::new("some_id", [1, 2], Cohort::from_hint("some-channel"));
        assert_eq!(app.id, "some_id");
        assert_eq!(app.version, [1, 2].into());
        assert_eq!(app.fingerprint, None);
        assert_eq!(app.cohort.hint, Some("some-channel".to_string()));
        assert_eq!(app.cohort.name, None);
        assert_eq!(app.cohort.id, None);
        assert_eq!(app.user_counting, UserCounting::ClientRegulatedByDate(None));
    }

    #[test]
    fn test_app_with_fingerprint() {
        let app = App::with_fingerprint(
            "some_id_2",
            [4, 6],
            "some_fp",
            Cohort::from_hint("test-channel"),
        );
        assert_eq!(app.id, "some_id_2");
        assert_eq!(app.version, [4, 6].into());
        assert_eq!(app.fingerprint, Some("some_fp".to_string()));
        assert_eq!(app.cohort.hint, Some("test-channel".to_string()));
        assert_eq!(app.cohort.name, None);
        assert_eq!(app.cohort.id, None);
        assert_eq!(app.user_counting, UserCounting::ClientRegulatedByDate(None));
    }

    #[test]
    fn test_app_load() {
        block_on(async {
            let mut storage = MemStorage::new();
            let json = serde_json::json!({
            "cohort": {
                "cohort": "some_id",
                "cohorthint":"some_hint",
                "cohortname": "some_name"
            },
            "user_counting": {
                "ClientRegulatedByDate":123
            }});
            let json = serde_json::to_string(&json).unwrap();
            let mut app = App::new("some_id", [1, 2], Cohort::new(""));
            storage.set_string(&app.id, &json).await.unwrap();
            app.load(&storage).await;

            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            assert_eq!(cohort, app.cohort);
            assert_eq!(UserCounting::ClientRegulatedByDate(Some(123)), app.user_counting);
        });
    }

    #[test]
    fn test_app_load_empty_storage() {
        block_on(async {
            let storage = MemStorage::new();
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            let mut app = App::new("some_id", [1, 2], cohort);
            app.user_counting = UserCounting::ClientRegulatedByDate(Some(123));
            app.load(&storage).await;

            // existing data not overwritten
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            assert_eq!(cohort, app.cohort);
            assert_eq!(UserCounting::ClientRegulatedByDate(Some(123)), app.user_counting);
        });
    }

    #[test]
    fn test_app_load_malformed() {
        block_on(async {
            let mut storage = MemStorage::new();
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            let mut app = App::new("some_id", [1, 2], cohort);
            storage.set_string(&app.id, "not a json").await.unwrap();
            app.user_counting = UserCounting::ClientRegulatedByDate(Some(123));
            app.load(&storage).await;

            // existing data not overwritten
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            assert_eq!(cohort, app.cohort);
            assert_eq!(UserCounting::ClientRegulatedByDate(Some(123)), app.user_counting);
        });
    }

    #[test]
    fn test_app_load_partial() {
        block_on(async {
            let mut storage = MemStorage::new();
            let json = serde_json::json!({
            "cohort": {
                "cohorthint":"some_hint_2",
                "cohortname": "some_name_2"
            },
            "user_counting": {
                "ClientRegulatedByDate":null
            }});
            let json = serde_json::to_string(&json).unwrap();
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            let mut app = App::new("some_id", [1, 2], cohort);
            storage.set_string(&app.id, &json).await.unwrap();
            app.user_counting = UserCounting::ClientRegulatedByDate(Some(123));
            app.load(&storage).await;

            // existing data not overwritten
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint_2".to_string()),
                name: Some("some_name_2".to_string()),
            };
            assert_eq!(cohort, app.cohort);
            assert_eq!(UserCounting::ClientRegulatedByDate(Some(123)), app.user_counting);
        });
    }

    #[test]
    fn test_app_persist() {
        block_on(async {
            let mut storage = MemStorage::new();
            let cohort = Cohort {
                id: Some("some_id".to_string()),
                hint: Some("some_hint".to_string()),
                name: Some("some_name".to_string()),
            };
            let mut app = App::new("some_id", [1, 2], cohort);
            app.user_counting = UserCounting::ClientRegulatedByDate(Some(123));
            app.persist(&mut storage).await;

            let expected = serde_json::json!({
            "cohort": {
                "cohort": "some_id",
                "cohorthint":"some_hint",
                "cohortname": "some_name"
            },
            "user_counting": {
                "ClientRegulatedByDate":123
            }});
            let json = storage.get_string(&app.id).await.unwrap();
            assert_eq!(expected, serde_json::Value::from_str(&json).unwrap());
            assert_eq!(false, storage.committed());
        });
    }

    #[test]
    fn test_app_persist_empty() {
        block_on(async {
            let mut storage = MemStorage::new();
            let cohort = Cohort { id: None, hint: None, name: None };
            let mut app = App::new("some_id", [1, 2], cohort);
            app.user_counting = UserCounting::ClientRegulatedByDate(None);
            app.persist(&mut storage).await;

            let expected = serde_json::json!({
            "cohort": {},
            "user_counting": {
                "ClientRegulatedByDate":null
            }});
            let json = storage.get_string(&app.id).await.unwrap();
            assert_eq!(expected, serde_json::Value::from_str(&json).unwrap());
            assert_eq!(false, storage.committed());
        });
    }

    #[test]
    fn test_app_get_current_channel() {
        let cohort = Cohort { name: Some("current-channel-123".to_string()), ..Cohort::default() };
        let app = App::new("some_id", [0, 1], cohort);
        assert_eq!("current-channel-123", app.get_current_channel());
    }

    #[test]
    fn test_app_get_current_channel_default() {
        let app = App::new("some_id", [0, 1], Cohort::default());
        assert_eq!("stable-channel", app.get_current_channel());
    }

    #[test]
    fn test_app_get_target_channel() {
        let cohort = Cohort::from_hint("target-channel-456");
        let app = App::new("some_id", [0, 1], cohort);
        assert_eq!("target-channel-456", app.get_target_channel());
    }

    #[test]
    fn test_app_get_target_channel_fallback() {
        let cohort = Cohort { name: Some("current-channel-123".to_string()), ..Cohort::default() };
        let app = App::new("some_id", [0, 1], cohort);
        assert_eq!("current-channel-123", app.get_target_channel());
    }

    #[test]
    fn test_app_get_target_channel_default() {
        let app = App::new("some_id", [0, 1], Cohort::default());
        assert_eq!("stable-channel", app.get_target_channel());
    }

    #[test]
    fn test_app_set_target_channel() {
        let mut app = App::new("some_id", [0, 1], Cohort::default());
        assert_eq!("stable-channel", app.get_target_channel());
        app.set_target_channel(Some("new-target-channel".to_string()));
        assert_eq!("new-target-channel", app.get_target_channel());
        app.set_target_channel(None);
        assert_eq!("stable-channel", app.get_target_channel());
    }

    #[test]
    #[should_panic]
    fn test_appset_panics_with_empty_vec() {
        AppSet::new(vec![]);
    }

    #[test]
    fn test_appset_update_from_omaha() {
        let mut app_set = AppSet::new(vec![App::new("some_id", [0, 1], Cohort::default())]);
        let cohort = Cohort { name: Some("some-channel".to_string()), ..Cohort::default() };
        let app_responses = vec![AppResponse {
            app_id: "some_id".to_string(),
            cohort: cohort.clone(),
            user_counting: UserCounting::ClientRegulatedByDate(None),
            result: Action::Updated,
        }];
        block_on(async {
            app_set.update_from_omaha(app_responses).await;
            assert_eq!(cohort, app_set.to_vec().await[0].cohort);
        });
    }

    #[test]
    fn test_appset_to_vec() {
        block_on(async {
            let app_set = AppSet::new(vec![App::new("some_id", [0, 1], Cohort::default())]);
            let mut vec = app_set.to_vec().await;
            vec[0].id = "some_other_id".to_string();
            assert_eq!("some_id", app_set.to_vec().await[0].id);
        });
    }
}
