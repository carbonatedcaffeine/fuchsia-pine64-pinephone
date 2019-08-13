//! Expectations for Bluetooth Peers (i.e. Remote Devices)

use fidl_fuchsia_bluetooth_control::{RemoteDevice, TechnologyType};

use crate::expectation::Predicate;

pub fn name(expected_name: &str) -> Predicate<RemoteDevice> {
    Predicate::equal(Some(expected_name.to_string()))
        .over(|peer: &RemoteDevice| &peer.name, ".name")
}
pub fn identifier(expected_ident: &str) -> Predicate<RemoteDevice> {
    let identifier = expected_ident.to_string();
    Predicate::<RemoteDevice>::predicate(
        move |peer| peer.identifier == identifier,
        format!("identifier == {}", expected_ident),
    )
}
pub fn address(expected_address: &str) -> Predicate<RemoteDevice> {
    let address = expected_address.to_string();
    Predicate::<RemoteDevice>::predicate(
        move |peer| peer.address == address,
        format!("address == {}", expected_address),
    )
}
pub fn technology(tech: TechnologyType) -> Predicate<RemoteDevice> {
    Predicate::<RemoteDevice>::predicate(
        move |peer| peer.technology == tech,
        format!("technology == {:?}", tech),
    )
}
pub fn connected(connected: bool) -> Predicate<RemoteDevice> {
    Predicate::<RemoteDevice>::predicate(
        move |peer| peer.connected == connected,
        format!("connected == {}", connected),
    )
}
pub fn bonded(bonded: bool) -> Predicate<RemoteDevice> {
    Predicate::<RemoteDevice>::predicate(
        move |peer| peer.bonded == bonded,
        format!("bonded == {}", bonded),
    )
}
