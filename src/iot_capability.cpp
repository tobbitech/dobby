#include <Arduino.h>

#include "iot_capability.h"

OnOffSwitch::OnOffSwitch(
            Connection * conn, 
            int pin, 
            etl::string<32> name, 
            etl::string<MQTT_TOPIC_STRING_LENGTH> mqtt_topic, 
            etl::string<8> on_value,
            etl::string<8> off_value
        )
{
    _conn = conn;
    _pin = pin;
    _name = name;
    _mqtt_topic = mqtt_topic;
    _on_value = on_value;
    _off_value = off_value;
}

void OnOffSwitch::begin() {
    pinMode(_pin, OUTPUT);
    _conn->subscribe_mqtt_topic(_mqtt_topic);
    log_info("OnOffSwitch \'%s\' created on topic %s", _name.c_str(), _mqtt_topic.c_str());
    _conn->maintain();
    // Register action
    _conn->register_action(_mqtt_topic, [this](etl::string<16> action_string) { 
        this->parse_action(action_string); 
    });

    
}

etl::string<MQTT_TOPIC_STRING_LENGTH> OnOffSwitch::getMqttTopic() {
    return(_mqtt_topic);
}

etl::string<32> OnOffSwitch::getName() {
    return(_name);
}

void OnOffSwitch::turnOn(bool updateOnOffTopic)
{
    digitalWrite(_pin, HIGH);
    if (updateOnOffTopic) { _conn->publish(_mqtt_topic, _on_value); }
    log_info("OnOffSwitch %s turned ON", _name.c_str() );
}


void OnOffSwitch::turnOff(bool updateOnOffTopic)
{
    digitalWrite(_pin, LOW);
    if (updateOnOffTopic) { _conn->publish(_mqtt_topic, _off_value); }
    log_info("OnOffSwitch %s turned OFF", _name.c_str());
}

void OnOffSwitch::toggle(bool updateOnOffTopic)
{
    int state = digitalRead(_pin);
    if (state == HIGH) {
        OnOffSwitch::turnOn(updateOnOffTopic);
    }
    else {
        OnOffSwitch::turnOff(updateOnOffTopic);
    }
}

void OnOffSwitch::setSwitchState(etl::string<8> on_off_value, bool updateOnOffTopic)
{
    if (on_off_value == _on_value) {
        OnOffSwitch::turnOn(updateOnOffTopic);
    }
    else if (on_off_value == _off_value)
    {
        OnOffSwitch::turnOff(updateOnOffTopic);
    }
    else
    {
        // a unexpected value was received. Log and do nothing.
        log_info("Unexpected switch state received: %s", on_off_value);
    }
}

etl::string<8> OnOffSwitch::getOnValue()
{
    return(_on_value);
}

etl::string<8> OnOffSwitch::getOffValue()
{
    return(_off_value);
}

void OnOffSwitch::parse_action(etl::string<16> action_string) {
    // action_string is MQTT message received on action topic
    if (action_string == _on_value) {
        turnOn();
    }
    else if (action_string == _off_value) {
        turnOff();
    }
    else {
        log_error("Cannot parse action string: %s", action_string);
    }
}



DS18B20_temperature_sensors::DS18B20_temperature_sensors(Connection * conn, int pin, etl::string<64> mqtt_main_topic): _oneWire(pin), _sensors(&_oneWire) 
// dunno why this works, but initating DallasTemperature objects in contructor does not work
{
    _conn = conn;
    _mqtt_main_topic = mqtt_main_topic;
    _numberOfDevices = 0;
    _mapSize = 0;
    _getting_data = false;
}

uint8_t DS18B20_temperature_sensors::scanForSensors()
{
    // returns the number of sensors found on the bus
    log_info("Scanning for DS18B20 sensors");
    _sensors.begin();
    _numberOfDevices = _sensors.getDeviceCount();
    log_info("Found %d DS18B20 sensors", _numberOfDevices);

    // reset address and name arrays
    for (int i = 0; i < _numberOfDevices; i++) {
        _deviceNames[i] = "";
        for (byte j = 0; j < 8; j++) {
            _deviceAddresses[i][j] = 0;
        }
    }

    // store address for all devices in _deviceAddresses
    for (int i = 0; i < _numberOfDevices; i++) {
        _sensors.getAddress(_deviceAddresses[i], i);
        if ( _deviceNames[i].length() == 0) {
            _deviceNames[i] = DS18B20_temperature_sensors::getAddressString(i);
            log_info("Devicename: %s", _deviceNames[i].c_str());
            for( int j = 0; j < _mapSize; j++) {
                if ( _deviceNames[i] == _addressMap[j]) {
                    _deviceNames[i] = _nameMap[j];
                    log_info("Mapped name %s to device address %s", _nameMap[j].c_str(), getAddressString(i).c_str());
                }
            }
        }
    }


    // _sensors.requestTemperatures();
    return(_numberOfDevices);
}

etl::string<24> DS18B20_temperature_sensors::convertAddressToString(DeviceAddress address)
{
    // Converts address of type DeviceAddress to String

    etl::string<24> deviceName;
    etl::string<3> byteString;
    for (byte i = 0; i < 8; i++) {
        byteString.clear();
        byteString.assign(etl::to_string(address[i], byteString, etl::format_spec().base(16).fill('0').width(2)));
        deviceName += byteString;
        // address[i];
        // deviceName[i] += String(address[i], HEX);
        if (i < 7) {
            deviceName += ":";
        }
    }
    return(deviceName);
}

etl::string<24> DS18B20_temperature_sensors::getAddressString(int deviceIndex){
    // Converts address of type DeviceAddress to String
    return(DS18B20_temperature_sensors::convertAddressToString(_deviceAddresses[deviceIndex]));
}

void DS18B20_temperature_sensors::mapNameToDeviceAddress(DeviceAddress address, etl::string<24> name)
{
    _addressMap[_mapSize] = convertAddressToString(address);
    _nameMap[_mapSize] = name;
    _mapSize++;
}

etl::string<64> DS18B20_temperature_sensors::getMqttTopic() {
    return(_mqtt_main_topic);
}

etl::string<24> DS18B20_temperature_sensors::getName(int deviceIndex) {
    return(_deviceNames[deviceIndex]);
}

void DS18B20_temperature_sensors::publishAllTemperatures()
{
   _getting_data = true;
   _currentDevice = 0;
   uint32_t request_start_time = millis();
   _sensors.requestTemperatures();
   uint32_t request_time_ms = millis() - request_start_time;
   log_info("Requested all sensors in %dms", request_time_ms);
}   


float DS18B20_temperature_sensors::getTemperature(uint8_t deviceIndex) {
    if (deviceIndex >= _numberOfDevices) {
        return(-200); // index out of range
    }
    _sensors.requestTemperatures();
    return(_sensors.getTempC(_deviceAddresses[deviceIndex]));
}

float DS18B20_temperature_sensors::getTemperatureByName(etl::string<24> deviceName) {
    for (uint8_t i = 0; i < _numberOfDevices; i++ ) {
        if (_deviceNames[i] == deviceName ) {
            return(getTemperature(i));
        }
    }
    return(-201); // deviceName not found
}

void DS18B20_temperature_sensors::_get_sensor_data_nonblocking() {
    int i = _currentDevice++;

    etl::string<88> deviceMqttTopic(_mqtt_main_topic);
    deviceMqttTopic += "/" ;
    deviceMqttTopic += _deviceNames[i];
    log_debug("Device name is: %s and topic is %s", _deviceNames[i], deviceMqttTopic);
    etl::string<16> temperature_string;
    temperature_string.assign(etl::to_string(_sensors.getTempC(_deviceAddresses[i]), temperature_string, etl::format_spec().precision(2)));
    _conn->publish(deviceMqttTopic, temperature_string);
    log_info("%s: %.2fC", _deviceNames[i].c_str(), _sensors.getTempC(_deviceAddresses[i]));

    if (_currentDevice >= _numberOfDevices ) {
        _getting_data = false;
    }
}

void DS18B20_temperature_sensors::tick() {
    if (_getting_data == true) {
        _get_sensor_data_nonblocking();
    }

}

InputMomentary::InputMomentary(
            Connection * conn, 
            int pin, 
            etl::string<32> name, 
            etl::string<MQTT_TOPIC_STRING_LENGTH> mqtt_topic,
            float analog_threshold_V,
            bool on_level,
            u_int32_t debounce_delay,
            etl::string<8> on_value, 
            etl::string<8> off_value)
{
    _conn = conn;
    _pin = pin;
    _mqtt_topic = mqtt_topic;
    // _mqtt_set_topic = mqtt_topic + "/set";
    _name = name;
    _analog_threshold_V = analog_threshold_V;
    _pressed = on_level;
    _unpressed = !on_level;
    _debounce_delay = debounce_delay;
    _on_value = on_value;
    _off_value = off_value;

    _state = InputMomentary::RESET;
    _last_state = InputMomentary::RESET;
    _last_debounce_time = 0;
    _is_pressed = false;
    _is_released = false;
    _sticky_timer.set(0, 's');
    _is_sticky_held = false;

}

void InputMomentary::begin() {
    // _conn->subscribe_mqtt_topic(_mqtt_set_topic);
    _conn->subscribe_mqtt_topic(_mqtt_topic);
    log_debug("Push button %s created on topic %s", _name.c_str(), _mqtt_topic.c_str());
}

etl::string<32> InputMomentary::get_name() {
    return(_name);
}

etl::string<64> InputMomentary::get_mqtt_topic() {
    return(_mqtt_topic);
}

void InputMomentary::set_sticky_button_timer(Timer sticky_timer) {
    _sticky_timer = sticky_timer;
}

uint32_t InputMomentary::get_remaining_sticky_hold_time_ms() {
    return(_sticky_timer.remaining() );
}

bool InputMomentary::is_pressed() {
    return _is_pressed;
}

bool InputMomentary::is_released() {
    return _is_released;
}

bool InputMomentary::is_held() {
    return _is_held;
}

bool InputMomentary::is_sticky_held() {
    return (_is_sticky_held);
}

u_int32_t InputMomentary::get_hold_time_ms() {
    return millis() -_hold_time_ms;
}

void InputMomentary::press() {
    _virtual_press = true;
}

void InputMomentary::tick() {
    switch_value = _unpressed;
    if (_virtual_press == true) {
        switch_value = _pressed;
    }
    else if (_analog_threshold_V == 0 ) {
        switch_value = digitalRead(_pin);
    }
    else {
        uint16_t value = analogRead(_pin);
        uint16_t threshold = round(4096 / 3.3) * _analog_threshold_V;
        if (value > threshold) {
            switch_value = _pressed;
        }
    }
    _last_state = _state;

    switch(_state) {
        case InputMomentary::RESET:
            _is_pressed = false;
            _is_held = false;
            _is_released = false;
            _hold_time_ms = millis();
            _state = InputMomentary::START;
            break;
        case InputMomentary::START:
            if (switch_value == _pressed) {
                _state = InputMomentary::GO;
            } 
            break;
        case InputMomentary::GO:
            _debounce_timer.set(_debounce_delay, 'm');
            _state = InputMomentary::WAIT;
            break;
        case InputMomentary::WAIT:
            if (switch_value == _unpressed) {
                _state = InputMomentary::RESET;
            } else if (_debounce_timer.is_done()) {
                _state = InputMomentary::TRIGGERED;
            }
            break;
        case InputMomentary::TRIGGERED:
            _virtual_press = false;
            _is_pressed = true;
            _hold_time_ms = millis();
            // Serial.println("Button pressed");
            _sticky_timer.reset();
            _conn->publish(_mqtt_topic, _on_value);
            _state = InputMomentary::HELD;
            break;

        case InputMomentary::HELD:
            _is_pressed = false;
            _is_held = true;
            _is_sticky_held = true;
            if (switch_value == _unpressed ) {
                _state = InputMomentary::STICKY;
            }
            break;
        case InputMomentary::STICKY:
            _is_held = false;
            if (_sticky_timer.is_done() ) {
                _state = InputMomentary::RELEASED;
            }
            break;
        case InputMomentary::RELEASED:
            _is_sticky_held = false;
            _is_released = true;
            _conn->publish(_mqtt_topic, _off_value);
            _state = InputMomentary::RESET;
            break;
    }
}




HANreader::HANreader(Connection * conn, etl::string<MQTT_TOPIC_STRING_LENGTH> mqttTopic, uint8_t RXpin, uint8_t TXpin): serialHAN(1)
{
    _RXpin = RXpin;
    _TXpin = TXpin;
    _conn = conn;
    _mqttTopic = mqttTopic;
    _han_hex_topic = mqttTopic;
    _han_hex_topic += "/hex";
}

void HANreader::begin() {
    serialHAN.begin(2400, SERIAL_8N1, _RXpin, _TXpin);
    _last_byte_millis = 0;
    // _message = "";
    _message_buf_pos = 0;
}

void HANreader::end() {
    serialHAN.end();
}

void HANreader::tick() {
    uint32_t time_since_last_byte = millis() - _last_byte_millis;

    if ( time_since_last_byte > HAN_READ_TIMEOUT_MS && _message_buf_pos > 0 ) {
        // parse_message( _message );
        _message_buf[_message_buf_pos] = '\0';
        parse_message();
        // _message = "";
        _message_buf_pos = 0;
    }

    if ( serialHAN.available() > 0 ) {
        char recv_char = serialHAN.read();
        _last_byte_millis = millis(); // reset timeout counter
        // if (recv_char != NULL) {
        // _message += recv_char;
        _message_buf[_message_buf_pos++] = recv_char;
            

        // }
    }
}

u_int16_t crc16x25(unsigned char *data_p, u_int16_t lenght) {
    // calculates CRC16/X25
    u_int16_t crc = 0xFFFF;
    u_int32_t data;
    u_int16_t crc16_table[] = {
            0x0000, 0x1081, 0x2102, 0x3183,
            0x4204, 0x5285, 0x6306, 0x7387,
            0x8408, 0x9489, 0xa50a, 0xb58b,
            0xc60c, 0xd68d, 0xe70e, 0xf78f
    };

    while(lenght--){
        crc = ( crc >> 4 ) ^ crc16_table[(crc & 0xf) ^ (*data_p & 0xf)];
        crc = ( crc >> 4 ) ^ crc16_table[(crc & 0xf) ^ (*data_p++ >> 4)];
    }

    data = crc;
    return (~crc);
}

void HANreader::parse_message() {
    for (int i = 0; i < _message_buf_pos; i++ ) {
            etl::string<2> hex_byte;
            etl::to_string(_message_buf[i], hex_byte, etl::format_spec().base(16).fill('0').width(2));
            _hex_message += hex_byte;
    }
    // publish raw HAN message
    
    _conn->publish(_han_hex_topic, _hex_message);
    log_debug("Published HAN hex message (length %d) to topic %s", _hex_message.size(),_han_hex_topic.c_str());
    _hex_message.clear();
    
    // parse HAN message
    size_t i = 0;
    if (_message_buf[i++] == 0x7e ) { 
        // flag found
        log_debug("HAN message flag found (0x7e)");
        u_int8_t header[6] = {
            _message_buf[i++],
            _message_buf[i++],
            _message_buf[i++],
            _message_buf[i++],
            _message_buf[i++],
            _message_buf[i++]
        };
        u_int16_t header_checksum = _message_buf[i++] | _message_buf[i++] << 8;
        u_int16_t calc_header_checksum = crc16x25(header, 6);
        if (header_checksum == calc_header_checksum ) {
            log_debug("Header checksum OK! %0x", header_checksum);
        } else {
            log_warning("Header checksum error. Dropping package. Got %0x, but expected %0x", header_checksum, calc_header_checksum);
            return;
        }
        
        return;

        // jump past next 9 bytes, we dont need them for anything
        i += 9; 

        int datatype = _message_buf[i++];
        int payload_lines = _message_buf[i++];

        // String name = "";
        // String unit = "";
        // String subtopic = "";
        // String value_str = "";
        _subtopic.clear();
        _value_string.clear();

        size_t current_line_index;

        for (int line = 0; line < payload_lines; line++) {
            i += 4; // jump past type identifier in line
            u_int8_t obis_code[6] = {
                _message_buf[i++],
                _message_buf[i++],
                _message_buf[i++],
                _message_buf[i++],
                _message_buf[i++],
                _message_buf[i++]
            };
        

            log_debug("OBIS code: %02x %02x %02x %02x %02x %02x\t", obis_code[0], obis_code[1], obis_code[2], obis_code[3], obis_code[4], obis_code[5] );

            for (size_t l = 0; l < _no_han_lines; l++) {
                if (std::equal(obis_code, obis_code + sizeof obis_code / sizeof *obis_code, han_lines[l].obis_code) ) {
                    current_line_index = l;
                    // name = han_lines[l].name;
                    // unit = han_lines[l].unit;
                    // subtopic = han_lines[l].subtopic;
                    log_debug("OBIS code found: %s subtopic: %s", han_lines[l].name.c_str(), han_lines[l].subtopic.c_str());
                    break;
                } else {
                    log_debug("No OBIS code found");
                }
            }

            u_int8_t variable_type = _message_buf[i++];

            if (variable_type == 0x0a ) {
                // this is a string, have to find length
                int string_length = _message_buf[i++];
                char string_contents[string_length+1];
                int j;
                for (j=0; j<string_length;j++) {
                     string_contents[j] = _message_buf[i++];
                }
                string_contents[j] = '\0';
                _value_string = string_contents;
            } else if (variable_type == 0x06) {
                // this is a uint32 -> Energy, cumulative energy
                u_int32_t value;
                value = _message_buf[i+3] | _message_buf[i+2] << 8 | _message_buf[i+1] << 16 | _message_buf[i] << 24;
                i += 4 + 6; // +6 is the stuff after the value on each line
                etl::to_string(value, _value_string);

                if (han_lines[current_line_index].name == "Cummulative active import" ||
                    han_lines[current_line_index].name == "Cummulative active export" ||
                    han_lines[current_line_index].name == "Cummulative reactive import" ||
                    han_lines[current_line_index].name == "Cummulative reactive export" ) {
                        float value_f = value / 100.0; // cumulative energy has resolution 0.01kWh
                        etl::to_string(value_f, _value_string, etl::format_spec().precision(2));
                }
            } else if (variable_type == 0x9) {
                // this is clock time - octet-string
                int string_length = _message_buf[i++];
                _value_string.clear();
                uint16_t year  = _message_buf[i+1] | _message_buf[i] << 8;
                i += 2;
                uint8_t  month = _message_buf[i++];
                uint8_t  day   = _message_buf[i++];
                uint8_t  dow   = _message_buf[i++];
                uint8_t  hour  = _message_buf[i++];
                uint8_t  minute= _message_buf[i++];
                uint8_t  second= _message_buf[i++];

                etl::to_string(year, _value_string);
                _value_string += ".";
                etl::to_string(month, _value_string, true);
                _value_string += ".";
                etl::to_string(day, _value_string, true);
                _value_string += "-";
                etl::to_string(hour, _value_string, true);
                _value_string += ":";
                etl::to_string(minute, _value_string, true);
                _value_string += ":";
                etl::to_string(second, _value_string, true);
                

                // value_str = String(year) + "." + String(month) + "." + String(day) + "-"
                //         + String(hour) + ":" + String(minute) + ":" + String(second) + " ";

                // for (int j = 8; j < string_length; j++) {
                //     uint8_t octet = _message_buf[i++];
                //     value_str += String(octet);
                //     if ( j < string_length -1 ) { 
                //         value_str += ".";
                //     }
                // }            
            } else if ( variable_type == 0x10 ){
                // this is a i16 -> Current
                int16_t value;
                value = _message_buf[i+1] | _message_buf[i] << 8;
                i += 2 + 6; // +6 is the stuff after the value on each line
                etl::to_string(value, _value_string);
                if (han_lines[current_line_index].name=="Current L1" 
                    || han_lines[current_line_index].name=="Current L2" 
                    || han_lines[current_line_index].name=="Current L3" 
                ) {
                    float value_f = value / 10.0; // Current has 0.1A resolution
                    etl::to_string(value_f, _value_string, etl::format_spec().precision(1));
                }
            } else if ( variable_type == 0x12 ) {
                // this is a u16 -> Voltage
                uint16_t value;
                value = _message_buf[i+1] | _message_buf[i] << 8;
                i += 2 + 6; // +6 is the stuff after the value on each line
                if (han_lines[current_line_index].name=="Voltage L1" 
                    || han_lines[current_line_index].name=="Voltage L2" 
                    || han_lines[current_line_index].name=="Voltage L3" 
                ) {
                    float value_f = value / 10.0; // Voltage has 0.1A resolution
                    etl::to_string(value_f, _value_string, etl::format_spec().precision(1));
                }
            }
            
            _subtopic.clear();
            _subtopic = _mqttTopic;
            _subtopic += "/" ;
            _subtopic += han_lines[current_line_index].subtopic;

            _conn->publish(_subtopic, _value_string );
        }

        // checking packet checksum
        u_int16_t packet_checksum = _message_buf[i++] | _message_buf[i++] << 8;
        u_int16_t calc_packet_checksum = crc16x25(_message_buf + 1, i-3);
        if (packet_checksum == calc_packet_checksum ) {
            // _conn->debug("Packet checksum OK! " + String(packet_checksum, HEX) );
        } else {
            log_warning("Packet checksum error. Dropping packet. Packet: Got %0x, but expected %0x", packet_checksum, calc_packet_checksum );
            return;
        }
        
        if (_message_buf[i] != 0x7e) {
            log_warning("No end flag found. Instead found: %0x. Dropping packet", _message_buf[i]);
        }
    }
}


// VEdirectReader::VEdirectReader(Connection * conn, String mqttTopic, uint8_t RXpin, uint8_t TXpin): serialVE(2)
// {
//     _RXpin = RXpin;
//     _TXpin = TXpin;
//     _conn = conn;
//     _mqttTopic = mqttTopic;
// }

// void VEdirectReader::begin() {
//     serialVE.begin(19200, SERIAL_8N1, _RXpin, _TXpin); // for hardwareserial
//     _send_raw_data_timer.set(100, "seconds");
//     set_publish_timer_s(5);
//     _last_byte_millis = 0;
//     _message = "";
//     _message_buf_pos = 0;
//     _voltage_V = 0;
//     _current_A = 0;
//     _power_W = 0;
//     _soc = 0;
//     _soc_by_v = 0;
//     _pv_voltage_V = 0;
//     _pv_power_W = 0;
//     _voltage_is_set = false;
//     _current_is_set = false;
//     _power_is_set = false;
//     _soc_is_set = false;
//     _pv_voltage_is_set = false;
//     _pv_power_is_set = false;
//     _yield_total_is_set = false;
//     _yield_today_is_set = false;
//     _max_power_today_is_set = false;
//     _yield_yesterday_is_set = false;
//     _max_power_yesterday_is_set = false;
    
// }

// void VEdirectReader::end() {
//     serialVE.end();
// }

// void VEdirectReader::tick() {
//     uint32_t time_since_last_byte = millis() - _last_byte_millis;

//     if ( time_since_last_byte > VEDIRECT_TIMEOUT_MS && _message != "" ) {
//         _message_buf[_message_buf_pos] = '\0';
//         parse_message();
//         _message = "";
//         _message_buf_pos = 0;
//     }

//     if ( serialVE.available() > 0 ) {
//         char recv_char = serialVE.read();
//         _last_byte_millis = millis(); // reset timeout counter
//         _message += recv_char;
//         _message_buf[_message_buf_pos++] = recv_char;
//     }
// }

// void VEdirectReader::set_publish_timer_s(u_int16_t seconds) {
//     _publish_data_timer.set(seconds, "seconds");
// }

// void VEdirectReader::publish_data() {
//     if (_voltage_is_set) {
//         _conn->publish(_mqttTopic + "/battery_voltage_V", String(_voltage_V, 2));
//         _conn->publish(_mqttTopic + "/soc_by_v", String(_soc_by_v, 1));
//     }
//     if (_current_is_set) {
//         _conn->publish(_mqttTopic + "/current_I", String(_current_A, 2));
//     }
//     if (_power_is_set) {
//         _conn->publish(_mqttTopic + "/power_W", String(_power_W, 0));
//     }
//     if (_soc_is_set) {
//         _conn->publish(_mqttTopic + "/soc_%", String(_soc, 1));
//     }
//     if (_pv_voltage_is_set) {
//         _conn->publish(_mqttTopic + "/pv_voltage_V", String(_pv_voltage_V, 2));
//     }
//     if (_pv_power_is_set) {
//         _conn->publish(_mqttTopic + "/pv_power_W", String(_pv_power_W, 0));
//     }
//     if (_yield_total_is_set) {
//         _conn->publish(_mqttTopic + "/yield_total_kWh", String(_yield_total_kWh, 2));
//     }
//     if (_yield_today_is_set) {
//         _conn->publish(_mqttTopic + "/yield_today_kWh", String(_yield_today_kWh, 2));
//     }
//     if (_max_power_today_is_set) {
//         _conn->publish(_mqttTopic + "/max_power_today_W", String(_max_power_today_W, 0));
//     }
//     if (_yield_yesterday_is_set) {
//         _conn->publish(_mqttTopic + "/yield_yesterday_kWh", String(_yield_yesterday_kWh, 2));
//     }
//     if (_max_power_yesterday_is_set) {
//         _conn->publish(_mqttTopic + "/max_power_yesterday_W", String(_max_power_yesterday_W, 0));
//     }
// }

// void VEdirectReader::parse_message() {
//     if( _send_raw_data_timer.is_done() ) {
//         _conn->publish(_mqttTopic + "/VEdirect/raw", _message);
//     }

//     String key = "none";
//     String value ="empty";

//     bool separator_found = false;

//     for (int i = 0; i < _message_buf_pos; i++) {
//         if ( _message[i] == '\n') {
//             if (key == "V") {
//                 _voltage_V = value.toInt() / 1000.0;
//                 _soc_by_v = (0.9369*_voltage_V*_voltage_V - 87.69*_voltage_V + 2050);
//                 if (_soc_by_v > 100 ) { _soc_by_v = 100; }
//                 _voltage_is_set = true;
//                 //trollslottetBatterySOCbyV.sendCommand((0.009369*Math.pow(x,2) - 0.8769*x + 20.5)*100);
// //88.69*Math.pow(x,6) - 151.5*Math.pow(x,5) - 21.37*Math.pow(x,4) + 179.9*Math.pow(x,3) - 125.7*Math.pow(x,2) + 39.33*x + 48);
//             }
//             else if (key == "I") {
//                 _current_A = value.toInt() / 1000.0;
//                 _current_is_set = true;
//             }
//             else if (key == "P") {
//                 _power_W = value.toInt();
//                 _power_is_set = true;
//             }
//             else if (key == "SOC") {
//                 _soc = value.toInt() / 10;
//                 _soc_is_set = true;
//             }
//             else if (key == "VPV") {
//                 _pv_voltage_V = value.toInt() / 1000.0;
//                 _pv_voltage_is_set = true;
//             }
//             else if (key == "PPV") {
//                 _pv_power_W = value.toInt();
//                 _pv_power_is_set = true;
//             }
//             else if (key == "H19") {
//                 _yield_total_kWh = value.toInt() / 100.0;
//                 _yield_total_is_set = true;
//             }
//             else if (key == "H20") {
//                 _yield_today_kWh = value.toInt() / 100.0;
//                 _yield_today_is_set = true;
//             }
//             else if (key == "H21") {
//                 _max_power_today_W = value.toInt();
//                 _max_power_today_is_set = true;
//             }
//             else if (key == "H22") {
//                 _yield_yesterday_kWh = value.toInt() / 100.0;
//                 _yield_yesterday_is_set = true;
//             }
//             else if (key == "H23") {
//                 _max_power_yesterday_W = value.toInt();
//                 _max_power_yesterday_is_set = true;
//             }

//             key = "";
//             value = "";
//             separator_found = false;
//         }
//         else if (_message[i] == '\t') {
//             // separator found!
//             separator_found = true;
//         }
//         else if (separator_found == false ) {
//             // get data field:
//             key += _message[i];
//         }
//         else {
//             value += _message[i];
//         }
//     }
// }


// Thermostat::Thermostat(Connection * conn, 
//         DS18B20_temperature_sensors * tempsensor,
//         String tempsensor_name,
//         uint8_t relay_pin,
//         String name, 
//         String mqtt_topic,
//         float target_temperature_C,
//         float hysteresis_C
//     ) {
//     _conn = conn;
//     _tempsensor = tempsensor;
//     _tempsensor_name = tempsensor_name;
//     _relay_pin = relay_pin;
//     _name = name;
//     _mqtt_topic = mqtt_topic;
//     _target_temperature_C = target_temperature_C;
//     _hysteresis_C = hysteresis_C;
//     _is_cooling = false;
//     // _last_is_cooling = true;
//     _state_changed = true;
//     _minimum_off_time = 60000; // milliseconds
// }

// void Thermostat::begin() {

//     _last_tick = millis();
//     pinMode(_relay_pin, OUTPUT);
//     _min_temperature_C = _target_temperature_C - _hysteresis_C;
//     _max_temperature_C = _target_temperature_C + _hysteresis_C;
//     _mqtt_target_temp_topic = _mqtt_topic + "/target_temperature_C";
//     _mqtt_cooling_state_topic = _mqtt_topic + "/state_cooling";
//     _conn->subscribeMqttTopic(_mqtt_target_temp_topic);
//     _conn->debug("Thermostat created with topic: " + _mqtt_topic);
//     _conn->debug("Temerature shall stay between " + String(_min_temperature_C) + "C and " + String(_max_temperature_C) + "C" );
// }

// void Thermostat::set_target_temperature_C(float temperature) {
//     _target_temperature_C = temperature;
//     _min_temperature_C = _target_temperature_C - _hysteresis_C;
//     _max_temperature_C = _target_temperature_C + _hysteresis_C;
//     return;
// }

// void Thermostat::set_mqtt_target_temp_topic(String topic) {
//     _mqtt_target_temp_topic = topic;
// }

// String Thermostat::get_mqtt_target_temp_topic() {
//     return(_mqtt_target_temp_topic);
// }

// String Thermostat::get_mqtt_main_topic() {
//     return(_mqtt_topic);
// }

// float Thermostat::get_target_temperature_C() {
//     return(_target_temperature_C);
// }

// float Thermostat::get_max_temperature_C() {
//     return(_max_temperature_C);
// }

// float Thermostat::get_min_temperature_C() {
//     return(_min_temperature_C);
// }

// float Thermostat::get_hysteresis_C() {
//     return(_hysteresis_C);
// }


// float Thermostat::get_measured_temperature_C() {
//     float temperature = _tempsensor->getTemperatureByName(_tempsensor_name);
//     return(temperature);
// }

// void Thermostat::parse_mqtt_message(String mqtt_message, String topic) {
//     // _conn->debug("Parsing topic: " + topic + " message: " + mqtt_message);

//     if (topic == _mqtt_target_temp_topic) {
//         float new_target_temperature = mqtt_message.toFloat();
//         set_target_temperature_C(new_target_temperature);
//         _conn->debug("Setting target temperature to " + String(new_target_temperature) + "°C");
//     }
// }

// bool Thermostat::is_cooling() {
//     return(_is_cooling);
// }

// void Thermostat::tick() {
//     // delay to avoid relay clapping
//     if (millis() > (_last_tick + _minimum_off_time)) {
//         _last_tick = millis();
//         float current_temperature = get_measured_temperature_C();

//         if (current_temperature < _min_temperature_C && _is_cooling == true) {
//             _is_cooling = false;
//             _state_changed = true;
//         }
//         else if (current_temperature > _max_temperature_C && _is_cooling == false) {
//             _is_cooling = true;
//             _state_changed = true;
//         }

//         if (_state_changed) {
//             _state_changed = false;
//             // _last_is_cooling = _is_cooling;
//             String debug_info = "T: " + String(current_temperature, 1) + " [" + String(_min_temperature_C, 1) + ", " + String(_max_temperature_C, 1) + "] cooling: " + String(_is_cooling);
//             if (_is_cooling) {
//                 // start cooling, close relay
//                 _conn->debug("Start cooling. " + debug_info);
//                 digitalWrite(_relay_pin, HIGH);
//                 _conn->publish(_mqtt_cooling_state_topic, "1");
//             }
//             else {
//                 // Stopp cooling, open relay
//                 _conn->debug("Stopped cooling. " + debug_info);
//                 digitalWrite(_relay_pin, LOW);
//                 _conn->publish(_mqtt_cooling_state_topic, "0");

//             }
//         }

//     }

// }



StepperMotorDoor::StepperMotorDoor(
        Connection * conn_ptr, 
        AccelStepper * stepper_ptr,
        etl::string<32> name, 
        etl::string<64> mqtt_topic,
        int pin_coil_enable
    )
{
    _conn = conn_ptr;
    _stepper = stepper_ptr;
    _name = name;
    _mqtt_topic = mqtt_topic;
    _pin_coil_enable = pin_coil_enable;
    _change_positive_direction = false;
}

void StepperMotorDoor::begin() {
    pinMode(_pin_coil_enable, OUTPUT);
    _stepper->setAcceleration(100.0);
    _stepper->setMaxSpeed(200.0);
    StepperMotorDoor::setStepsToOpen(50);
    _stepper->setEnablePin(_pin_coil_enable);
    _conn->subscribe_mqtt_topic(_mqtt_topic);
    log_info("Stepper motor %s created on topic %s", _name.c_str(), _mqtt_topic.c_str() );
    // register action
    _conn->register_action(_mqtt_topic, [this](etl::string<16> action_string) { 
        this->parse_action(action_string); 
    });
}

etl::string<64> StepperMotorDoor::getMqttTopic() {
    return(_mqtt_topic);
}

etl::string<32> StepperMotorDoor::getName() {
    return(_name);
}

void StepperMotorDoor::tick() {
    // run this function as often as possible in main loop
    if (_open_limit_switch != nullptr && _open_limit_switch->is_pressed()) {
        log_info("Open limit switch pressed for stepper %s. Stopping.", _name.c_str());
        _stepper->stop();
        close_steps(100); // move a bit away from the switch to avoid it being pressed at startup
    }

    _stepper->run();
     if (_stepper->distanceToGo() == 0)
        // disable output
        digitalWrite(_pin_coil_enable, LOW);
}

void StepperMotorDoor::moveToStep(uint16_t step) {
    //enable output
    digitalWrite(_pin_coil_enable, HIGH);
    _stepper->moveTo(step);
    // _stepper->disableOutputs();
    log_info("Moved stepper %s to step %d", _name.c_str(), step);
}

void StepperMotorDoor::open_steps(uint16_t steps) {
    //enable output
    digitalWrite(_pin_coil_enable, HIGH);
    if ( _change_positive_direction ) {
        steps = (-1) * steps;
    }
    _stepper->moveTo(steps);
    log_info("Opened %s by moving %d steps", _name.c_str(), steps);
}

void StepperMotorDoor::close_steps(uint16_t steps) {
    //enable output
    digitalWrite(_pin_coil_enable, HIGH);
    if ( _change_positive_direction ) {
        steps = (-1) * steps;
    }
    _stepper->moveTo(-steps);
    log_info("Closed %s by moving %d steps", _name.c_str(), steps);
}

void StepperMotorDoor::open() {
    StepperMotorDoor::moveToStep(_steps_to_open);
    log_info("Opened %s", _name.c_str() );
}

void StepperMotorDoor::close() {
    StepperMotorDoor::moveToStep(0);
    log_info("Closed %s", _name.c_str());
}

void StepperMotorDoor::setStepsToOpen(uint16_t steps) {
    _steps_to_open = steps;
    if ( _change_positive_direction ) {
        _steps_to_open = (-1) * steps;
    }
}

void StepperMotorDoor::setAcceleration(float acceleration) {
    _stepper->setAcceleration(acceleration);
    log_info("Set acceleration of stepper %s to %.2f", _name.c_str(), acceleration);
}

void StepperMotorDoor::setMaxSpeed(float speed) {
    _stepper->setMaxSpeed(speed);
    log_info("Set max speed of stepper %s to %.2f", _name.c_str(), speed);
}

void StepperMotorDoor::resetInClosedPosition() {
    _stepper->setCurrentPosition(0);
    log_info("Reset stepper %s to closed position (step 0)", _name.c_str());
}

long StepperMotorDoor::getCurrentPosition() {
    long pos = _stepper->currentPosition();
    return(pos);
}

void StepperMotorDoor::changeDirection() {
    _change_positive_direction = !_change_positive_direction;
    log_info("Changed direction of stepper %s to %s.", _name.c_str(), _change_positive_direction ? "negative" : "positive");
}

void StepperMotorDoor::parse_action(etl::string<16> action_string) {
        // action_string is MQTT message received on action topic
    if (action_string == "reset_open") {
        setStepsToOpen(getCurrentPosition());
        return;
    }

    if (action_string == "reset_closed") {
        resetInClosedPosition();
        return;
    }


    if (action_string == "open") {
        open();
        return;
    }

    if (action_string == "close") {
        close();
        return;
    }

    float percent_open = etl::to_arithmetic<float>(action_string);
    log_debug("Received float value %.2f", percent_open);
    if (percent_open < 0.0 || percent_open > 1.0) {
        log_error("Percent open outside valid range [0, 1]: %.2f", percent_open);
        return;
    }
    else {
        uint16_t step_to_move_to = round(_steps_to_open*percent_open);
        moveToStep(step_to_move_to);
        return;
    }

    log_error("Cannot parse action string: %s", action_string);
    return;
}

void StepperMotorDoor::connect_open_limit_switch(InputMomentary * open_limit_switch) {
    _open_limit_switch = open_limit_switch;
    log_info("Connected top limit %s switch to stepper %s", _open_limit_switch->get_name().c_str(), _name.c_str());
}