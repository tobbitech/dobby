#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"
#include "iot_capability.h"
#include "boards/shroom.h"

// Connection details
// setting default values
#define MQTT_MAIN_TOPIC "madlasto"
#define CLIENT_NAME "drivhus"
#define MQTT_TOPIC MQTT_MAIN_TOPIC "/" CLIENT_NAME
#define DEFAULT_MQTT_HOST "192.168.2.4"
#define DEFAULT_MQTT_PORT 1883

// Custom pin definitions
#define WALLSWITCH_LEFT 48 // green wire
#define WALLSWITCH_RIGHT 45 // white wire

CommandParser cmd;
Connection conn;

// Create all Iot capability objects
DS18B20_temperature_sensors temperature_sensors(&conn, TEMP1_PIN, MQTT_TOPIC "/temperatures_C");
OnOffSwitch output_A(&conn, MOSFET_A, "output A", MQTT_TOPIC "/outputs/output_a");
OnOffSwitch output_B(&conn, MOSFET_B, "output B", MQTT_TOPIC "/outputs/output_b");
OnOffSwitch output_C(&conn, MOSFET_C, "output C", MQTT_TOPIC "/outputs/output_c");
OnOffSwitch output_D(&conn, MOSFET_D, "output D", MQTT_TOPIC "/outputs/output_d");
OnOffSwitch output_E(&conn, MOSFET_E, "output E", MQTT_TOPIC "/outputs/output_e");

OnOffSwitch *onOffSwitches[] = {&output_A, &output_B, &output_C, &output_D, &output_E};
size_t noOnOffSwitches = sizeof(onOffSwitches) / sizeof(onOffSwitches[0]);

InputMomentary push1(&conn, PUSH_BUTTON_1, "Push 1", MQTT_TOPIC "/inputs/push1");
InputMomentary push2(&conn, PUSH_BUTTON_2, "Push 2", MQTT_TOPIC "/inputs/push2");
InputMomentary push3(&conn, PUSH_BUTTON_3, "Push 3", MQTT_TOPIC "/inputs/push3");
InputMomentary push4(&conn, PUSH_BUTTON_4, "Push 4", MQTT_TOPIC "/inputs/push4");
InputMomentary wallSwitchLeft(&conn, WALLSWITCH_LEFT, "Wallswitch left", MQTT_TOPIC "/inputs/wallswitch_left", 0, LOW);
InputMomentary wallSwitchRight(&conn, WALLSWITCH_RIGHT, "Wallswitch right", MQTT_TOPIC "/inputs/wallswitch_right", 0, LOW);

InputMomentary *pushButtons[] = {&push1, &push2, &push3, &push4, &wallSwitchLeft, &wallSwitchRight};
size_t noPushButtons = 6;

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
  DeviceAddress onboardSensor =       {0x28, 0xFF, 0xB2, 0x42, 0xBB, 0x22, 0x02, 0xF2};
  temperature_sensors.mapNameToDeviceAddress(onboardSensor, "greenhouse_inside");

  temperature_sensors.scanForSensors();
  temperature_sensors.publishAllTemperatures();

  for (int i=0; i<noOnOffSwitches; i++) {
    onOffSwitches[i]->begin();
  } 

  push1.begin();
  push2.begin();
  push3.begin();
  push4.begin();
  push3.set_sticky_button_timer(Timer(10, 's'));
  push4.set_sticky_button_timer(Timer(5, 's'));
  wallSwitchLeft.begin();
  wallSwitchRight.begin();
  wallSwitchLeft.set_sticky_button_timer(Timer(5, 's'));
  wallSwitchRight.set_sticky_button_timer(Timer(10, 's'));

  temperature_sensors.scanForSensors();
  temperature_sensors.publishAllTemperatures();

  push1.begin();
}

Timer telemetry_interval_timer(1, 'M');
Timer send_network_info_timer(20, 's');

void loop()
{
  cmd.tick();
  conn.maintain();
  push1.tick();
  push2.tick();
  push3.tick();
  push4.tick();
  wallSwitchLeft.tick();
  wallSwitchRight.tick();
  temperature_sensors.tick();

  if (telemetry_interval_timer.is_done())
  {
    log_debug("Publishing all temperatures");
    temperature_sensors.publishAllTemperatures();
  }
}
