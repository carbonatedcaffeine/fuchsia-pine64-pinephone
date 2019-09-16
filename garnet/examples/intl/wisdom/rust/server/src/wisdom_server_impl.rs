// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use {
    failure::Fail, fidl_fuchsia_intl as fintl, rust_icu_common as ucommon, rust_icu_sys as usys,
    rust_icu_ucal as ucal, rust_icu_udat as udat, rust_icu_uloc as uloc,
    rust_icu_ustring as ustring, std::convert::TryFrom,
};

/// All error classes produced in this module.
#[derive(Fail, Debug)]
pub enum Error {
    /// There was an error in the underlying ICU library.
    #[fail(display = "ICU common error: {}", _0)]
    Common(ucommon::Error),
}
impl From<ucommon::Error> for Error {
    fn from(e: ucommon::Error) -> Self {
        Error::Common(e)
    }
}

// TODO(fmil): We should use the server host time zone, but the C API for that is only available
// starting from as of yet unreleased ICU 65.
const UNKNOWN_TIMEZONE: &'static str = "Etc/Unknown";

// Creates a calendar from the supplied ID and time zone.
fn calendar_from(calendar_id: &str, time_zone_id: &str) -> Result<ucal::UCalendar, Error> {
    // UCAL_DEFAULT means "whatever calendar applies for the locale".
    ucal::UCalendar::new(time_zone_id, calendar_id, usys::UCalendarType::UCAL_DEFAULT)
        .map_err(|e| e.into())
}

/// Returns the wise response for the time on the timestamp, in all the specified locales and
/// calendars.
fn build_response(
    timestamp_ms: usys::UDate,
    locale_ids: &Vec<&String>,
    calendars: &Vec<ucal::UCalendar>,
    tz_id: &ustring::UChar,
) -> Result<String, Error> {
    let pattern: ustring::UChar = ustring::UChar::try_from("")?;
    let time_style = usys::UDateFormatStyle::UDAT_FULL;
    let date_style = usys::UDateFormatStyle::UDAT_FULL;

    // TODO(fmil): I18N I18ize this response.
    let mut response = String::from("\nA wise one knows the time...\n\n");

    for locale in locale_ids {
        for calendar in calendars {
            let loc_ref: &str = *locale;
            let loc = uloc::ULoc::try_from(loc_ref)?;
            let mut fmt = udat::UDateFormat::new(time_style, date_style, &loc, tz_id, &pattern)?;
            fmt.set_calendar(calendar);
            let formatted_date = fmt.format(timestamp_ms)?;
            response.push_str(&format!("{}\n", formatted_date));
        }
    }

    // TODO(fmil): I18N I18ize this response.
    response.push_str("\nBut is it the 𝒄𝒐𝒓𝒓𝒆𝒄𝒕  time?\n");
    Ok(response)
}

/// Serves the responses to the "ask for wisdom" requests.
pub fn ask_for_wisdom(intl_profile: &fintl::Profile, timestamp_ms: i64) -> Result<String, Error> {
    // Excavate the identifiers, which are just strings.
    //
    // This long statement deserves some explanation, though.
    // `as_ref()` is needed to treat locales as a reference and avoid a move.
    // `unwrap()` is needed because locales are for some reason wrapped in Option<> even though
    // they are required.
    // `iter()` is neede because unwrapped locales are an array, which by default aren't iterable
    // without an explicit call to `iter`.
    // `map()` turns the compound object into a simple string
    // collect() will produce a vector once all transforms are done.
    //
    // Note that all operations that use `&str` reuse references to strings that exist somewhere
    // inside the intl_profile struct.
    let locale_ids: Vec<_> = intl_profile.locales.as_ref().unwrap().iter().map(|l| &l.id).collect();

    let time_zone_ids: Vec<_> =
        intl_profile.time_zones.as_ref().unwrap().iter().map(|t| &t.id).collect();
    let time_zone: &str = match time_zone_ids.len() {
        // If unspecified, use a default timezone.
        0 => UNKNOWN_TIMEZONE,
        // If specified, use the first (most preferred) timezone.
        _ => &time_zone_ids[0],
    };

    // Parse the requested calendar IDs, using the first requested time zone, or as fallback the
    // device time zone.
    let calendar_ids = intl_profile.calendars.as_ref().unwrap();
    let calendars: Vec<_> = match calendar_ids.len() {
        // If no calendars have been requested explicitly, manufacture one based on the time zone
        // and the passed locale ID.
        0 => vec![calendar_from(locale_ids[0], time_zone)?],
        // In all other cases, produce calendars from the passed in calendar IDs.
        _ => calendar_ids
            .iter()
            .map(|c| calendar_from(&c.id, time_zone).expect("could not create a calendar"))
            .collect(),
    };

    // Now that we prepared all the input parameters (whew!), we build the long response string
    // ... or I guess fail with an error.
    let tz_id = ustring::UChar::try_from(time_zone)?;
    build_response(timestamp_ms as usys::UDate, &locale_ids, &calendars, &tz_id)
}

#[cfg(test)]
mod tests {
    use super::*;
    use icu_data;

    #[test]
    fn basic() -> Result<(), super::Error> {
        // Keep the ICU data loader live for the duration of the test.  It is never "used" except
        // for its ability to mark that we want ICU data to be present.
        let _loader = icu_data::Loader::new().expect("ICU data is loaded");

        struct Test {
            profile: fintl::Profile,
            timestamp_ms: i64,
            result: String,
        };
        let tests = vec![
            Test {
                profile: fintl::Profile {
                    locales: Some(vec![
                        fintl::LocaleId { id: "sr-RS".to_string() },
                        fintl::LocaleId { id: "nl-NL".to_string() },
                    ]),
                    calendars: Some(vec![fintl::CalendarId {
                        id: "und-u-ca-gregorian".to_string(),
                    }]),
                    temperature_unit: Some(fintl::TemperatureUnit::Celsius),
                    time_zones: Some(vec![]),
                },
                timestamp_ms: 0,
                result: vec![
                    "\nA wise one knows the time...\n\n",
                    "четвртак, 01. јануар 1970. 00:00:00 GMT\n",
                    "donderdag 1 januari 1970 om 00:00:00 GMT",
                    "\n\nBut is it the 𝒄𝒐𝒓𝒓𝒆𝒄𝒕  time?\n",
                ]
                .concat()
                .to_string(),
            },
            Test {
                profile: fintl::Profile {
                    locales: Some(vec![
                        fintl::LocaleId { id: "sr-RS".to_string() },
                        fintl::LocaleId { id: "nl-NL".to_string() },
                    ]),
                    calendars: Some(vec![fintl::CalendarId {
                        id: "und-u-ca-gregorian".to_string(),
                    }]),
                    temperature_unit: Some(fintl::TemperatureUnit::Celsius),
                    time_zones: Some(vec![fintl::TimeZoneId {
                        id: "America/Los_Angeles".to_string(),
                    }]),
                },
                timestamp_ms: 100000000, // About a day after the Unix Epoch
                result: vec![
                   "\nA wise one knows the time...\n\n",
                   "четвртак, 01. јануар 1970. 19:46:40 Северноамеричко пацифичко стандардно време\n",
                   "donderdag 1 januari 1970 om 19:46:40 Pacific-standaardtijd",
                   "\n\nBut is it the 𝒄𝒐𝒓𝒓𝒆𝒄𝒕  time?\n",
                ]
                .concat()
                .to_string(),
            },
            Test {
                profile: fintl::Profile {
                    locales: Some(vec![
                        fintl::LocaleId { id: "en-US".to_string() },
                    ]),
                    calendars: Some(vec![fintl::CalendarId {
                        id: "und-u-ca-hebrew".to_string(),
                    }]),
                    temperature_unit: Some(fintl::TemperatureUnit::Celsius),
                    time_zones: Some(vec![fintl::TimeZoneId {
                        id: "America/New_York".to_string(),
                    }]),
                },
                timestamp_ms: 100000000, // About a day after the Unix Epoch
                result: vec![
                   "\nA wise one knows the time...\n\n",
                   "Thursday, Tevet 23, 5730 at 10:46:40 PM Eastern Standard Time",
                   "\n\nBut is it the 𝒄𝒐𝒓𝒓𝒆𝒄𝒕  time?\n",
                ]
                .concat()
                .to_string(),
            },
            Test {
                profile: fintl::Profile {
                    locales: Some(vec![
                        fintl::LocaleId { id: "ar-AU-u-ca-hebrew-fw-tuesday-nu-traditio-tz-usnyc".to_string() },
                    ]),
                    calendars: Some(vec![fintl::CalendarId {
                        id: "und-u-ca-islamic".to_string(),
                    }]),
                    temperature_unit: Some(fintl::TemperatureUnit::Celsius),
                    time_zones: Some(vec![fintl::TimeZoneId {
                        id: "America/New_York".to_string(),
                    }]),
                },
                timestamp_ms: 100000000, // About a day after the Unix Epoch
                result: vec![
                   "\nA wise one knows the time...\n\n",
                   "الخميس، ٢٣ شوال ١٣٨٩ هـ ١٠:٤٦:٤٠ م التوقيت الرسمي الشرقي لأمريكا الشمالية",
                   "\n\nBut is it the 𝒄𝒐𝒓𝒓𝒆𝒄𝒕  time?\n",
                ]
                .concat()
                .to_string(),
            },
        ];
        for t in tests {
            let result = ask_for_wisdom(&t.profile, t.timestamp_ms)?;
            assert_eq!(t.result, result);
        }
        Ok(())
    }
}
