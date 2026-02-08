#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"

CommandParser cmd;
Connection conn;

void callback(char *callbackTopic, byte *payload, unsigned int payloadLength);

void setup() {

  set_log_level(log_severity::DEBUG);

  pinMode(39, OUTPUT); // fix dim LED

  Serial.begin(112500);
  Serial.println("Hi there!");

  cmd.add(1, list_commands, "Lists available commands");
  cmd.add(2, reboot, "Reboots device");
  cmd.add(3, status, "Shows status of device");
  cmd.add(4, enable_ota, "Sets device in OTA mode for 60 seconds");

  conn.connect(
    WIFI_SSID,
    WIFI_PW,
    "192.168.2.4",
    "test",
    callback,
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

void callback(char *callbackTopic, byte *payload, unsigned int payloadLength) {
  conn.number_mqtt_callbacks++;
  if ( conn.new_mqtt_message ) {
    // ignore message while the previous message is handled
    return;
  }

  for (size_t i = 0; i < payloadLength; i++ ) {
    conn.received_mqtt_message.push_back( (char)payload[i] );
  }
  conn.received_mqtt_topic.assign(callbackTopic);
  conn.new_mqtt_message = true;
}