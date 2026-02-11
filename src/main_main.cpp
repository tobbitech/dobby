#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"
#include "iot_capability.h"
#include "boards/dino.h"

#define MAINTOPIC "test"

CommandParser cmd;
Connection conn;

OnOffSwitch led1(&conn, LED_GAUGE_OK, "LED 1", MAINTOPIC "/led1");
OnOffSwitch led2(&conn, LED_GAUGE_H1, "LED 2", MAINTOPIC "/led2");
OnOffSwitch led3(&conn, LED_GAUGE_H2, "LED 3", MAINTOPIC "/led3");
OnOffSwitch led4(&conn, LED_GAUGE_H3, "LED 4", MAINTOPIC "/led4");
OnOffSwitch led5(&conn, LED_GAUGE_H4, "LED 5", MAINTOPIC "/led5");

InputMomentary boot_sw(&conn, BOOT_SWITCH_PIN, "Boot", MAINTOPIC "/button1");

void setup() {
  set_log_level(log_severity::DEBUG);
  pinMode(39, OUTPUT); // fix dim LED
  Serial.begin(112500);

  cmd.add(1, CMD::list_commands, "Lists available commands");
  cmd.add(2, CMD::reboot, "Reboot device");
  cmd.add(3, CMD::status, "Shows status of device");
  cmd.add(4, CMD::enable_ota, "Sets device in OTA mode for 60 seconds");
  cmd.add(5, CMD::set_log_level, "Set log level. Arg1: DEBUG INFO WARNING ERROR CRITICAL RESPONSE");
  cmd.add(6, CMD::log_ip, "Show device IP address");
  cmd.add(7, CMD::log_mac, "Show device hardware address");

  conn.connect( WIFI_SSID,
    WIFI_PW,
    "192.168.2.4",
    MAINTOPIC,
    "",
    "",
    "",
    1883,
    "TestClient",
    21,
    47,
    false
  );

  led1.begin();
  led2.begin();
  led3.begin();
  led4.begin();
  led5.begin();

  
}

void loop() {
  cmd.tick();
  conn.maintain();

  boot_sw.tick();
}