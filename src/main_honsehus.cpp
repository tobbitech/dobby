#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"
#include "mqttConnection.h"
#include "wifi_cred.h"
#include "iot_capability.h"
// #include "boards/shroom.h"
#include "boards/dino.h"
// Connection details
// setting default values

#define MQTT_MAIN_TOPIC "madlasto"
#define CLIENT_NAME "honsehus"
#define MQTT_TOPIC MQTT_MAIN_TOPIC "/" CLIENT_NAME
#define DEFAULT_MQTT_HOST "192.168.2.4"
#define DEFAULT_MQTT_PORT 1883

// custom pins
#define STEPPER_COIL_A1 11
#define STEPPER_COIL_A2 12
#define STEPPER_COIL_B1 13
#define STEPPER_COIL_B2 14
// #define STEPPER_ENABLE 46
#define STEPPER_ENABLE LED_TEST_HIGH_LOAD_OK

#define LED_WIFI LED_TEST_CABLE_OK
#define LED_MQTT LED_TEST_NO_LOAD_OK

CommandParser cmd;
Connection conn;

AccelStepper stepper(AccelStepper::FULL4WIRE, STEPPER_COIL_A1, STEPPER_COIL_A2, STEPPER_COIL_B1, STEPPER_COIL_B2);
StepperMotorDoor chickendoor(&conn, &stepper, "Chickendoor", MQTT_TOPIC "/door", STEPPER_ENABLE);

// Create all Iot capability objects
// DS18B20_temperature_sensors temperature_sensors(&conn, TEMP1_PIN, MQTT_TOPIC "/temperatures_C");

// OnOffSwitch output_A(&conn, MOSFET_A, "output A", MQTT_TOPIC "/outputs/output_a");
// OnOffSwitch output_B(&conn, MOSFET_B, "output B", MQTT_TOPIC "/outputs/output_b");
// OnOffSwitch output_C(&conn, MOSFET_C, "output C", MQTT_TOPIC "/outputs/output_c");
// OnOffSwitch output_D(&conn, MOSFET_D, "output D", MQTT_TOPIC "/outputs/output_d");
// OnOffSwitch output_E(&conn, MOSFET_E, "output E", MQTT_TOPIC "/outputs/output_e");

// OnOffSwitch *onOffSwitches[] = {&output_A, &output_B, &output_C, &output_D, &output_E};
// size_t noOnOffSwitches = sizeof(onOffSwitches) / sizeof(onOffSwitches[0]);

// InputMomentary push1(&conn, PUSH_BUTTON_1, "push 1", MQTT_TOPIC "/inputs/push1");
// InputMomentary push2(&conn, PUSH_BUTTON_2, "push 2", MQTT_TOPIC "/inputs/push2");
// InputMomentary push3(&conn, PUSH_BUTTON_3, "push 3", MQTT_TOPIC "/inputs/push3");
// InputMomentary push4(&conn, PUSH_BUTTON_4, "push 4", MQTT_TOPIC "/inputs/push4");

InputMomentary top_stop(&conn, BOOT_SWITCH_PIN, "top stop", MQTT_TOPIC "/inputs/top_stop", 0, 0);
void setup() {
  // pinMode(MOSFET_A, OUTPUT);
  // pinMode(MOSFET_B, OUTPUT);
  // pinMode(MOSFET_C, OUTPUT);
  // pinMode(MOSFET_D, OUTPUT);
  // pinMode(MOSFET_E, OUTPUT);
  // pinMode(LED_WIFI, OUTPUT);
  // pinMode(LED_MQTT, OUTPUT);
  // pinMode(LED_STATUS, OUTPUT);
  // pinMode(PUSH_BUTTON_1, INPUT);
  // pinMode(PUSH_BUTTON_2, INPUT);
  // pinMode(PUSH_BUTTON_3, INPUT);
  // pinMode(PUSH_BUTTON_4, INPUT);
\

  Serial.begin(115200);

  cmd.add(1, CMD::list_commands, "Lists available commands");
  cmd.add(2, CMD::reboot, "Reboot device");
  cmd.add(3, CMD::status, "Shows status of device");
  cmd.add(4, CMD::enable_ota, "Sets device in OTA mode for 60 seconds");
  cmd.add(5, CMD::set_log_level, "Set log level. Arg1: DEBUG INFO WARNING ERROR CRITICAL RESPONSE");
  cmd.add(6, CMD::log_ip, "Show device IP address");
  cmd.add(7, CMD::log_mac, "Show device hardware address");

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

  // initialize outputs:
  // for (size_t i = 0; i < noOnOffSwitches; i++)
  // {
  //   onOffSwitches[i]->begin();
  // }

  // map temperature sensor names:
  // DeviceAddress onboardSensor = {0x28, 0xFF, 0x67, 0x7B, 0xBB, 0x22, 0x02, 0xA5};

  // temperature_sensors.mapNameToDeviceAddress(onboardSensor, "hønsehus");

  // temperature_sensors.scanForSensors();
  // temperature_sensors.publishAllTemperatures();

  // push1.begin();
  // push2.begin();
  // push3.begin();
  // push4.begin();
  top_stop.begin();

  chickendoor.begin();
  chickendoor.connect_open_limit_switch(&top_stop);
  chickendoor.setAcceleration(50);
  chickendoor.setMaxSpeed(150);
  chickendoor.setStepsToOpen(2000);
  chickendoor.open();
}

Timer telemetry_interval_timer(1, 'M');
Timer send_network_info_timer(20, 's');
\
void loop()
{
  cmd.tick();
  conn.maintain();
  // push1.tick();
  // push2.tick();
  // push3.tick();
  // push4.tick();
  top_stop.tick();
  chickendoor.tick();
  // temperature_sensors.tick();

  // if (telemetry_interval_timer.is_done())
  // {
  //   log_info("Publishing all temperatures");
  //   temperature_sensors.publishAllTemperatures();
  // }
}
