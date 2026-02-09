#pragma once

#include <Arduino.h>
#include "logging.h"
#include "mqttConnection.h"
// #include <OneWire.h>
// #include <DallasTemperature.h>
// #include <HardwareSerial.h>

class OnOffSwitch
{
    public:
        OnOffSwitch(
            Connection * conn, 
            int pin, 
            etl::string<32> name, 
            etl::string<64> mqtt_topic, 
            etl::string<8> on_value = "true",
            etl::string<8> off_value = "false"
        );

        void begin();
        etl::string<64> getMqttTopic();
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
        etl::string<64> _mqtt_topic;
        etl::string<20> _mac_address;
        etl::string<8> _on_value;
        etl::string<8> _off_value;
};

// class DS18B20_temperature_sensors
// {
//     public:
//         DS18B20_temperature_sensors(Connection * conn, int pin, String mqtt_main_topic);
//         String getMqttTopic();
//         String getName(int deviceIndex);
//         String convertAddressToString(DeviceAddress address);
//         String getAddressString(int deviceIndex);
//         uint8_t scanForSensors();
//         float getTemperature(uint8_t deviceIndex);
//         float getTemperatureByName(String deviceName);
//         void mapNameToDeviceAddress(DeviceAddress address, String name);
//         void publishAllTemperatures();
//         void tick();

//     private:
//         Connection * _conn;
//         OneWire _oneWire;
//         DallasTemperature _sensors;
//         DeviceAddress _deviceAddresses[127];
//         String _deviceNames[127];
//         uint8_t _numberOfDevices;
//         uint8_t _currentDevice;
//         String _name;
//         String _mqtt_main_topic;
//         String _addressMap[127];
//         String _nameMap[127];
//         size_t _mapSize;
//         void _get_sensor_data_nonblocking();
//         bool _getting_data;
// };

// class InputMomentary {
//     public:
//         InputMomentary(
//             Connection * conn, 
//             int pin, 
//             String name, 
//             String mqtt_topic, 
//             float analog_threshold_V = 0, // uses digitalRead when 0
//             bool on_level = HIGH,
//             u_int32_t debounce_delay = 50,
//             String on_value = "true", 
//             String off_value = "false");
//         void begin();
//         void tick();
//         bool is_pressed();
//         bool is_held();
//         bool is_released();
//         u_int32_t get_hold_time_ms();
//         void set_sticky_button_timer(Timer sticky_timer);
//         bool is_sticky_held();
//         u_int32_t get_remaining_sticky_hold_time_ms();
//         String get_set_topic();
//         String get_name();
//         void press();

//         enum state {
//             RESET,
//             START,
//             GO,
//             WAIT,
//             TRIGGERED,
//             HELD,
//             STICKY,
//             RELEASED
//         };

//     private:
//         Connection * _conn;
//         int _pin;
//         String _name;
//         String _mqtt_topic;
//         String _mqtt_set_topic;
//         int _pressed;
//         int _unpressed;
//         u_int32_t _debounce_delay;
//         String _on_value;
//         String _off_value;
//         float _analog_threshold_V = 0;
        
//         int _state;
//         int _last_state;
//         u_int32_t _last_debounce_time;
//         u_int32_t _hold_time_ms;
//         Timer _debounce_timer;
//         Timer _sticky_timer;

//         int switch_value;
//         bool _is_pressed;
//         bool _is_held;
//         bool _is_released; 
//         bool _is_sticky_held;
//         bool _virtual_press = false;
// };

// #define HAN_READ_TIMEOUT_MS 100
// #define HAN_MAX_MESSAGE_SIZE 2000

// class HANreader {
//     public:
//         HANreader(Connection * conn, String mqttTopic, uint8_t RXpin, uint8_t TXpin);
//         void begin();
//         void end();
//         void tick();
//         HardwareSerial serialHAN;
//         void parse_message(String message);
//         void parse_message();

//         struct han_line {
//             u_int8_t obis_code[6];
//             String name;
//             String unit;
//             String topic;
//         };

//     private:
//         Connection * _conn;
//         String _mqttTopic;
//         uint8_t _RXpin;
//         uint8_t _TXpin;
//         int16_t _state;
//         int16_t _prev_state;
//         char _recv_char;
//         String _message;
//         uint8_t _message_buf[HAN_MAX_MESSAGE_SIZE];
//         uint16_t _message_buf_pos;
//         void _receive_char();
//         uint32_t _last_byte_millis;
//         bool _match_sequence(uint16_t);
//         u_int16_t _no_han_lines;
// };

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