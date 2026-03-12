#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"
#include "iot_capability.h"
#include "boards/r2d2.h"

// Connection details
// setting default values
#define MQTT_MAIN_TOPIC "trollslottet"
#define CLIENT_NAME "stue"
#define MQTT_TOPIC MQTT_MAIN_TOPIC "/" CLIENT_NAME
#define DEFAULT_MQTT_HOST "cederlov.com"
#define DEFAULT_MQTT_PORT 38883

CommandParser cmd;
Connection conn;

// Create all Iot capability objects
DS18B20_temperature_sensors temperature_sensors(&conn, TEMP1_PIN, MQTT_TOPIC "/temperatures_C");
InputMomentary push1(&conn, PUSH_BUTTON_1, "push", MQTT_TOPIC "/push");
VEdirectReader battery_monitor(&conn, MQTT_TOPIC "/battery", VEDIRECT_RX_OPTO, UART2_TX);

void pub_temps(CommandArgs args) {
  temperature_sensors.publishAllTemperatures();
}

void setup() {
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_MQTT, OUTPUT);

  Serial.begin(115200);

  cmd.add(1, CMD::list_commands, "Lists available commands");
  cmd.add(2, CMD::reboot, "Reboot device");
  cmd.add(3, CMD::status, "Shows status of device");
  cmd.add(4, CMD::enable_ota, "Sets device in OTA mode for 60 seconds");
  cmd.add(5, CMD::set_log_level, "Set log level. Arg1: DEBUG INFO WARNING ERROR CRITICAL RESPONSE");
  cmd.add(6, CMD::log_ip, "Show device IP address");
  cmd.add(7, CMD::log_mac, "Show device hardware address");
  cmd.add(8, pub_temps, "Publish all temperatures");

  conn.connect( 
      WIFI_SSID,
      WIFI_PW,
      DEFAULT_MQTT_HOST,
      MQTT_TOPIC,
      "",
      "",
      "",
      DEFAULT_MQTT_PORT,
      CLIENT_NAME,
      LED_WIFI,
      LED_MQTT,
      false
    );

    set_log_level(log_severity::DEBUG);

  // map temperature sensor names:
  DeviceAddress boardSensor =     {0x28, 0x6F, 0x93, 0x79, 0x97, 0x08, 0x03, 0xFD};
  temperature_sensors.mapNameToDeviceAddress(boardSensor, "inside");
  DeviceAddress wiredSensor =     {0x28, 0xAA, 0xF9, 0x29, 0x1A, 0x13, 0x02, 0x1E};
  temperature_sensors.mapNameToDeviceAddress(wiredSensor, "outside");

  temperature_sensors.scanForSensors();
  temperature_sensors.publishAllTemperatures();

  battery_monitor.begin();
  push1.begin();
}

Timer telemetry_interval_timer(1, 'M');
Timer send_network_info_timer(20, 's');

void loop()
{
  cmd.tick();
  conn.maintain();
  push1.tick();
  temperature_sensors.tick();
  battery_monitor.tick();

  if (telemetry_interval_timer.is_done())
  {
    log_debug("Publishing all temperatures");
    temperature_sensors.publishAllTemperatures();
  }
}
