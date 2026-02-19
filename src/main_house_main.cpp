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
#define CLIENT_NAME "house-main"
#define MQTT_TOPIC MQTT_MAIN_TOPIC "/" CLIENT_NAME
#define DEFAULT_MQTT_HOST "192.168.2.4"
#define DEFAULT_MQTT_PORT 1883

// custom pins
#define UART1_TXD 17
#define UART1_RXD 18
#define DOORBELL_PIN 8

CommandParser cmd;
Connection conn;
HANreader hanreader(&conn, MQTT_TOPIC "/han", UART1_RXD, UART1_TXD);

// Create all Iot capability objects
DS18B20_temperature_sensors temperature_sensors(&conn, TEMP1_PIN, MQTT_TOPIC "/temperatures_C");

OnOffSwitch output_A(&conn, MOSFET_A, "output A", MQTT_TOPIC "/outputs/output_a");
OnOffSwitch output_B(&conn, MOSFET_B, "output B", MQTT_TOPIC "/outputs/output_b");
OnOffSwitch output_C(&conn, MOSFET_C, "output C", MQTT_TOPIC "/outputs/output_c");
OnOffSwitch output_D(&conn, MOSFET_D, "output D", MQTT_TOPIC "/outputs/output_d");
OnOffSwitch output_E(&conn, MOSFET_E, "output E", MQTT_TOPIC "/outputs/output_e");

OnOffSwitch *onOffSwitches[] = {&output_A, &output_B, &output_C, &output_D, &output_E};
size_t noOnOffSwitches = sizeof(onOffSwitches) / sizeof(onOffSwitches[0]);

InputMomentary push1(&conn, PUSH_BUTTON_1, "push 1", MQTT_TOPIC "/inputs/push1");
InputMomentary push2(&conn, PUSH_BUTTON_2, "push 2", MQTT_TOPIC "/inputs/push2");
InputMomentary push3(&conn, PUSH_BUTTON_3, "push 3", MQTT_TOPIC "/inputs/push3");
InputMomentary push4(&conn, PUSH_BUTTON_4, "push 4", MQTT_TOPIC "/inputs/push4");
InputMomentary doorbell(&conn, DOORBELL_PIN, "doorbell", MQTT_TOPIC "/inputs/doorbell", 0, 0);

void setup() {
  pinMode(MOSFET_A, OUTPUT);
  pinMode(MOSFET_B, OUTPUT);
  pinMode(MOSFET_C, OUTPUT);
  pinMode(MOSFET_D, OUTPUT);
  pinMode(MOSFET_E, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_MQTT, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  pinMode(PUSH_BUTTON_1, INPUT);
  pinMode(PUSH_BUTTON_2, INPUT);
  pinMode(PUSH_BUTTON_3, INPUT);
  pinMode(PUSH_BUTTON_4, INPUT);
  pinMode(DOORBELL_PIN, INPUT);

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

  hanreader.begin();

  // initialize outputs:
  for (size_t i = 0; i < noOnOffSwitches; i++)
  {
    onOffSwitches[i]->begin();
  }

  // map temperature sensor names:
  DeviceAddress onboardSensor = {0x28, 0xFF, 0x67, 0x7B, 0xBB, 0x22, 0x02, 0xA5};
  DeviceAddress kitchen = {0x28, 0xF0, 0xF7, 0xA4, 0x0D, 0x00, 0x00, 0x2C};
  DeviceAddress cinema = {0x28, 0x6B, 0xD1, 0xA4, 0x0D, 0x00, 0x00, 0x01};
  DeviceAddress hall_upstairs = {0x28, 0xDE, 0xF7, 0xA4, 0x0D, 0x00, 0x00, 0x89};
  DeviceAddress knittingroom = {0x28, 0xFE, 0x00, 0xA5, 0x0D, 0x00, 0x00, 0x47};
  DeviceAddress tv_room = {0x28, 0x3F, 0xE6, 0xA4, 0x0D, 0x00, 0x00, 0x26};
  DeviceAddress wardrobe = {0x28, 0x02, 0xF8, 0xA4, 0x0D, 0x00, 0x00, 0x5F};
  DeviceAddress livingroom = {0x28, 0x3D, 0xF6, 0xA4, 0x0D, 0x00, 0x00, 0x34};
  DeviceAddress bathroom_upstairs = {0x28, 0x62, 0x7C, 0x5D, 0x0E, 0x00, 0x00, 0x4E};
  DeviceAddress washroom = {0x28, 0x7D, 0x7C, 0x5D, 0x0E, 0x00, 0x00, 0x31};
  DeviceAddress hall_downstairs = {0x28, 0xA6, 0xF7, 0xA4, 0x0D, 0x00, 0x00, 0xB0};
  DeviceAddress nerderom = {0x28, 0xBA, 0xF7, 0xA4, 0x0D, 0x00, 0x00, 0x96};
  DeviceAddress kjellerstue = {0x28, 0x01, 0xF8, 0xA4, 0x0D, 0x00, 0x00, 0x06};
  DeviceAddress markus = {0x28, 0xFF, 0x00, 0xA5, 0x0D, 0x00, 0x00, 0x70};
  DeviceAddress alva = {0x28, 0x5D, 0xF6, 0xA4, 0x0D, 0x00, 0x00, 0xF7};

  temperature_sensors.mapNameToDeviceAddress(kitchen, "kitchen");
  temperature_sensors.mapNameToDeviceAddress(cinema, "cinema");
  temperature_sensors.mapNameToDeviceAddress(hall_upstairs, "hall_upstairs");
  temperature_sensors.mapNameToDeviceAddress(knittingroom, "knittingroom");
  temperature_sensors.mapNameToDeviceAddress(tv_room, "tv_area");
  temperature_sensors.mapNameToDeviceAddress(wardrobe, "wardrobe");
  temperature_sensors.mapNameToDeviceAddress(livingroom, "living_room");
  temperature_sensors.mapNameToDeviceAddress(bathroom_upstairs, "bathroom_upstairs");
  temperature_sensors.mapNameToDeviceAddress(washroom, "washroom");
  temperature_sensors.mapNameToDeviceAddress(hall_downstairs, "hall_downstairs");
  temperature_sensors.mapNameToDeviceAddress(nerderom, "nerdroom");
  temperature_sensors.mapNameToDeviceAddress(kjellerstue, "kjellerstue");
  temperature_sensors.mapNameToDeviceAddress(markus, "markus");
  temperature_sensors.mapNameToDeviceAddress(alva, "alva");
  temperature_sensors.mapNameToDeviceAddress(onboardSensor, "router_cabinet");

  temperature_sensors.scanForSensors();
  temperature_sensors.publishAllTemperatures();

  push1.begin();
  push2.begin();
  push3.begin();
  push4.begin();
  doorbell.set_sticky_button_timer(Timer(2, 's'));
  doorbell.begin();
}

// Timer read_temperature_timer(10, "seconds");
Timer telemetry_interval_timer(1, 'M');
Timer send_network_info_timer(20, 's');


void loop()
{
  cmd.tick();
  conn.maintain();
  hanreader.tick();
  push1.tick();
  push2.tick();
  push3.tick();
  push4.tick();
  doorbell.tick();
  temperature_sensors.tick();

  if (telemetry_interval_timer.is_done())
  {
    log_info("Publishing all temperatures");
    temperature_sensors.publishAllTemperatures();
  }
}
