#pragma once

#include <Arduino.h>
#include "logging.h"
#include "mqttConnection.h"
#include "timer.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HardwareSerial.h>

#define MQTT_TOPIC_STRING_LENGTH 64
#define MQTT_PAYLOAD_STRING_LENGTH 256

class OnOffSwitch
{
    public:
        OnOffSwitch(
            Connection * conn, 
            int pin, 
            etl::string<32> name, 
            etl::string<MQTT_TOPIC_STRING_LENGTH> mqtt_topic, 
            etl::string<8> on_value = "true",
            etl::string<8> off_value = "false"
        );

        void begin();
        etl::string<MQTT_TOPIC_STRING_LENGTH> getMqttTopic();
        etl::string<32> getName();
        void turnOn(bool updateOnOfTopic = false);
        void turnOff(bool updateOnOfTopic = false);
        void toggle(bool updateOnOfTopic = false);
        void setSwitchState(etl::string<8> on_off_value, bool updateOnOfTopic = false);
        etl::string<8> getOnValue();
        etl::string<8> getOffValue();
        void parse_action(etl::string<16> action_string);


    private:
        Connection * _conn;
        int _pin;
        etl::string<32> _name;
        etl::string<MQTT_TOPIC_STRING_LENGTH> _mqtt_topic;
        etl::string<20> _mac_address;
        etl::string<8> _on_value;
        etl::string<8> _off_value;
};

class DS18B20_temperature_sensors
{
    public:
        DS18B20_temperature_sensors(Connection * conn, int pin, etl::string<MQTT_TOPIC_STRING_LENGTH> mqtt_main_topic);
        etl::string<MQTT_TOPIC_STRING_LENGTH> getMqttTopic();
        etl::string<24> getName(int deviceIndex);
        etl::string<24> convertAddressToString(DeviceAddress address);
        etl::string<24> getAddressString(int deviceIndex);
        uint8_t scanForSensors();
        float getTemperature(uint8_t deviceIndex);
        float getTemperatureByName(etl::string<24> deviceName);
        void mapNameToDeviceAddress(DeviceAddress address, etl::string<24> name);
        void publishAllTemperatures();
        void tick();

    private:
        Connection * _conn;
        OneWire _oneWire;
        DallasTemperature _sensors;
        DeviceAddress _deviceAddresses[127];
        etl::string<32> _deviceNames[127];
        uint8_t _numberOfDevices;
        uint8_t _currentDevice;
        etl::string<32> _name;
        etl::string<MQTT_TOPIC_STRING_LENGTH> _mqtt_main_topic;
        etl::string<24> _addressMap[127];
        etl::string<24> _nameMap[127];
        size_t _mapSize;
        void _get_sensor_data_nonblocking();
        bool _getting_data;
};

class InputMomentary {
    public:
        InputMomentary(
            Connection * conn, 
            int pin, 
            etl::string<32> name, 
            etl::string<MQTT_TOPIC_STRING_LENGTH> mqtt_topic, 
            float analog_threshold_V = 0, // uses digitalRead when 0
            bool on_level = HIGH,
            u_int32_t debounce_delay = 50,
            etl::string<8> on_value = "true", 
            etl::string<8> off_value = "false");
        void begin();
        void tick();
        bool is_pressed();
        bool is_held();
        bool is_released();
        u_int32_t get_hold_time_ms();
        void set_sticky_button_timer(Timer sticky_timer);
        bool is_sticky_held();
        u_int32_t get_remaining_sticky_hold_time_ms();
        etl::string<MQTT_TOPIC_STRING_LENGTH> get_mqtt_topic();
        etl::string<32> get_name();
        void press();

        enum state {
            RESET,
            START,
            GO,
            WAIT,
            TRIGGERED,
            HELD,
            STICKY,
            RELEASED
        };

    private:
        Connection * _conn;
        int _pin;
        etl::string<32> _name;
        etl::string<MQTT_TOPIC_STRING_LENGTH> _mqtt_topic;
        etl::string<MQTT_TOPIC_STRING_LENGTH> _mqtt_set_topic;
        int _pressed;
        int _unpressed;
        u_int32_t _debounce_delay;
        etl::string<8> _on_value;
        etl::string<8> _off_value;
        float _analog_threshold_V = 0;
        
        int _state;
        int _last_state;
        u_int32_t _last_debounce_time;
        u_int32_t _hold_time_ms;
        Timer _debounce_timer;
        Timer _sticky_timer;

        int switch_value;
        bool _is_pressed;
        bool _is_held;
        bool _is_released; 
        bool _is_sticky_held;
        bool _virtual_press = false;
};

#define HAN_READ_TIMEOUT_MS 100
#define HAN_MAX_MESSAGE_SIZE 2000

class HANreader {
    public:
        HANreader(Connection * conn, etl::string<MQTT_TOPIC_STRING_LENGTH> mqttTopic, uint8_t RXpin, uint8_t TXpin);
        void begin();
        void end();
        void tick();
        HardwareSerial serialHAN;
        void parse_message(String message);
        void parse_message();

        struct han_line {
            u_int8_t obis_code[6];
            etl::string<32> name;
            etl::string<16> unit;
            etl::string<32> subtopic;
        };

        // define all OBIS codes and corresponding topics here
        han_line version {.obis_code = { 0x01, 0x01, 0x00, 0x02, 0x81, 0xff }, .name="OBIS list version", .unit="", .subtopic="obis_list_version"  };
        han_line id {.obis_code = { 0x00, 0x00, 0x60, 0x01, 0x00, 0xff }, .name="Meter ID", .unit="", .subtopic="meter_id"  };
        han_line type {.obis_code = { 0x00, 0x00, 0x60, 0x01, 0x07, 0xff }, .name="Meter type", .unit="", .subtopic="meter_type"  };
        
        han_line active_import {.obis_code = { 0x01, 0x00, 0x01, 0x07, 0x00, 0xff }, .name="Active import", .unit="W", .subtopic="active_import_W"  };
        han_line active_export {.obis_code = { 0x01, 0x00, 0x02, 0x07, 0x00, 0xff }, .name="Active export", .unit="W", .subtopic="active_export_W"  };
        han_line reactive_import {.obis_code = { 0x01, 0x00, 0x03, 0x07, 0x00, 0xff }, .name="Reactive import", .unit="VAr", .subtopic="reactive_import_VAr"  };
        han_line reactive_export {.obis_code = { 0x01, 0x00, 0x04, 0x07, 0x00, 0xff }, .name="Reactive export", .unit="VAr", .subtopic="reactive_export_VAr"  };

        han_line current_L1 {.obis_code = { 0x01, 0x00, 0x1f, 0x07, 0x00, 0xff }, .name="Current L1", .unit="A", .subtopic="current_l1_A"  };
        han_line current_L2 {.obis_code = { 0x01, 0x00, 0x33, 0x07, 0x00, 0xff }, .name="Current L2", .unit="A", .subtopic="current_l2_A"  };
        han_line current_L3 {.obis_code = { 0x01, 0x00, 0x47, 0x07, 0x00, 0xff }, .name="Current L3", .unit="A", .subtopic="current_l3_A"  };

        han_line voltage_L1 {.obis_code = { 0x01, 0x00, 0x20, 0x07, 0x00, 0xff }, .name="Voltage L1", .unit="V", .subtopic="voltage_l1_V"  };
        han_line voltage_L2 {.obis_code = { 0x01, 0x00, 0x34, 0x07, 0x00, 0xff }, .name="Voltage L2", .unit="V", .subtopic="voltage_l2_V"  };
        han_line voltage_L3 {.obis_code = { 0x01, 0x00, 0x48, 0x07, 0x00, 0xff }, .name="Voltage L3", .unit="V", .subtopic="voltage_l3_V"  };

        han_line meter_clock {.obis_code = { 0x00, 0x00, 0x01, 0x00, 0x00, 0xff }, .name="Clock", .unit="", .subtopic="clock"  };
    
        han_line cum_active_import {.obis_code =   { 0x01, 0x00, 0x01, 0x08, 0x00, 0xff }, .name="Cummulative active import", .unit="kWh", .subtopic="cum_active_import_kWh"  };
        han_line cum_active_export {.obis_code =   { 0x01, 0x00, 0x02, 0x08, 0x00, 0xff }, .name="Cummulative active export", .unit="kWh", .subtopic="cum_active_export_kWh"  };
        han_line cum_reactive_import {.obis_code = { 0x01, 0x00, 0x03, 0x08, 0x00, 0xff }, .name="Cummulative reactive import", .unit="kVArh", .subtopic="cum_reactive_import_kVArh"  };
        han_line cum_reactive_export {.obis_code = { 0x01, 0x00, 0x04, 0x08, 0x00, 0xff }, .name="Cummulative reactive export", .unit="kVArh", .subtopic="cum_reactive_export_kVArh"  };

        etl::vector<han_line, 20> han_lines = {version, id, type, active_import, active_export, reactive_import, reactive_export, current_L1, current_L2, current_L3, 
                            voltage_L1, voltage_L2, voltage_L3, meter_clock, cum_active_import, cum_active_export, cum_reactive_import, cum_reactive_export };
        // _no_han_lines = 18;

    private:
        Connection * _conn;
        etl::string<MQTT_TOPIC_STRING_LENGTH> _mqttTopic;
        uint8_t _RXpin;
        uint8_t _TXpin;
        int16_t _state;
        int16_t _prev_state;
        char _recv_char;
        etl::string<MQTT_PAYLOAD_STRING_LENGTH> _message;
        uint8_t _message_buf[HAN_MAX_MESSAGE_SIZE];
        uint16_t _message_buf_pos;
        void _receive_char();
        uint32_t _last_byte_millis;
        bool _match_sequence(uint16_t);
        u_int16_t _no_han_lines;
        etl::string<32> _value_string;
        etl::string<88> _subtopic;
};

// #define VEDIRECT_TIMEOUT_MS 100
// #define VEDIRECT_MESSAGE_SIZE 2000
// #define VEDIRECT_NUMBER_KEYS_TO_PARSE 20
// class VEdirectReader {
//     public:
//         VEdirectReader(Connection *conn, String mqttTopic, u_int8_t RXpin, u_int8_t TXpin);
//         void begin();
//         void end();
//         void tick();
//         HardwareSerial serialVE;
//         void parse_message();
//         void set_publish_timer_s(u_int16_t seconds);
//         void publish_data();

//     private:
//         Connection * _conn;
//         String _mqttTopic;
//         uint8_t _RXpin;
//         uint8_t _TXpin;
//         int16_t _state;
//         int16_t _prev_state;
//         char _recv_char;
//         String _message;
//         uint8_t _message_buf[VEDIRECT_MESSAGE_SIZE];
//         uint16_t _message_buf_pos;
//         void _receive_char();
//         uint32_t _last_byte_millis;
//         Timer _send_raw_data_timer;
//         Timer _publish_data_timer;
//         float _voltage_V;
//         float _current_A;
//         float _power_W;
//         float _soc;
//         float _soc_by_v;
//         float _pv_voltage_V;
//         float _pv_power_W;
//         float _yield_total_kWh;
//         float _yield_today_kWh;
//         float _max_power_today_W;
//         float _yield_yesterday_kWh;
//         float _max_power_yesterday_W;
//         bool _voltage_is_set;
//         bool _current_is_set;
//         bool _power_is_set;
//         bool _soc_is_set;
//         bool _pv_voltage_is_set;
//         bool _pv_power_is_set;
//         bool _yield_total_is_set;
//         bool _yield_today_is_set;
//         bool _max_power_today_is_set;
//         bool _yield_yesterday_is_set;
//         bool _max_power_yesterday_is_set;
// };


// class Thermostat
// {
//     public:
//         Thermostat(Connection * conn, 
//             DS18B20_temperature_sensors * tempsensor, 
//             String tempsensor_name, 
//             uint8_t relay_pin,
//             String name, 
//             String mqtt_topic,
//             float _target_temperature_C = 4.0,
//             float hysteresis_C = 1.0
//         );
//         void begin();
//         void tick();
//         void set_target_temperature_C(float temperature);
//         float get_target_temperature_C();
//         float get_hysteresis_C();
//         float get_min_temperature_C();
//         float get_max_temperature_C();
//         float get_measured_temperature_C();
//         void set_mqtt_target_temp_topic(String topic);
//         String get_mqtt_main_topic();
//         String get_mqtt_target_temp_topic();
//         bool is_cooling();
//         void parse_mqtt_message(String mqtt_message, String topic); // sets min or max temp

//     private:
//         Connection * _conn;
//         DS18B20_temperature_sensors * _tempsensor;
//         String _tempsensor_name;
//         uint8_t _relay_pin;
//         uint8_t _pwm_on_value;
//         String _name;
//         String _mqtt_topic;
//         String _mqtt_target_temp_topic;
//         String _mqtt_cooling_state_topic;
//         float _target_temperature_C;
//         float _hysteresis_C;
//         float _max_temperature_C;
//         float _min_temperature_C;
//         uint32_t _last_tick;
//         bool _is_cooling;
//         // bool _last_is_cooling;
//         bool _state_changed;
//         uint32_t _minimum_off_time;

// };