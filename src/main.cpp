#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"

CommandParser cmd;
Connection conn;

void setup() {

  set_log_level(log_severity::DEBUG);

  pinMode(39, OUTPUT); // fix dim LED

  Serial.begin(112500);
  Serial.println("Hi there!");

  cmd.add(1, CMD::list_commands, "Lists available commands");
  cmd.add(2, CMD::reboot, "Reboot device");
  cmd.add(3, CMD::status, "Shows status of device");
  cmd.add(4, CMD::enable_ota, "Sets device in OTA mode for 60 seconds");
  cmd.add(5, CMD::set_log_level, "Set log level. Arg1: DEBUG INFO WARNING ERROR CRITICAL RESPONSE");
  cmd.add(6, CMD::log_ip, "Show device IP address");
  cmd.add(7, CMD::log_mac, "Show device hardware address");

  conn.connect( WIFI_SSID,
    WIFI_PW,
    "192.168.2.7",
    "test",
    "",
    "",
    "",
    1883,
    "TestClient",
    21,
    47,
    false
  );
}

void loop() {
  cmd.tick();
  conn.maintain();
  
}