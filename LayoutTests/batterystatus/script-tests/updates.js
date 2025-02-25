description('Tests that updates to the battery event causes new events to fire.');

var charging = false;
var chargingTime = Number.POSITIVE_INFINITY;
var dischargingTime = 6000;
var level = 0.7;

var battery = navigator.webkitBattery;

function checkBatteryStatus() {
    shouldBe('battery.charging', 'charging');
    shouldBe('battery.chargingTime', 'chargingTime');
    shouldBe('battery.dischargingTime', 'dischargingTime');
    shouldBe('battery.level', 'level');
}

function setBatteryStatus() {
    internals.setBatteryStatus(document, 'chargingchange', charging, chargingTime, dischargingTime, level);
}

function firstListener() {
    checkBatteryStatus();
    battery.removeEventListener('chargingchange', firstListener);
    battery.addEventListener('chargingchange', updateListener);
    charging = true;
    chargingTime = 7000;
    dischargingTime = Number.POSITIVE_INFINITY;
    level = 0.3;
	setBatteryStatus();
}

function updateListener(event) {
    checkBatteryStatus(event);
    finishJSTest();
}

battery.addEventListener('chargingchange', firstListener);
setBatteryStatus();
window.jsTestIsAsync = true;
