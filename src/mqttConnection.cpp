
#include "mqttConnection.h"

extern CommandParser cmd;

Connection::Connection()
{
    _wifi_ok = false;
    _mqtt_ok = false;

    new_mqtt_message = false;
    number_mqtt_callbacks = 0;
     _last_number_of_callbacks = 0;
    received_mqtt_topic.clear();
    received_mqtt_message.clear();
}

void Connection::set_wifi_ssid(etl::string<64> ssid) {
    _ssid = ssid;
}
void Connection::set_wifi_passwd(etl::string<64> passwd){
    _passwd = passwd;
}
void Connection::set_mqtt_host(etl::string<64> host){
    _host = host;
}
void Connection::set_mqtt_port(int port) {
    _port = port;
}
void Connection::set_mqtt_port(etl::string<64> port) {
    _port = etl::to_arithmetic<uint16_t>(port);
}
void Connection::set_mqtt_client_name(etl::string<64> client_name) {
    _client_name = client_name;
}

etl::string<64> Connection::get_mqtt_client_name() {
    return(_client_name);
}

// void set_ssl_ca(etl::string<64> ca) {
//     _wifi_client.setCACert(ca);
// }

// void setSslCert(String cert) {
//     _wifi_client.setCertificate(cert);
// }

// void setSslKey(String key) {
//     _wifi_client.setCertificate(key);
// }

void Connection::set_mqtt_main_topic(etl::string<64> main_topic) {
    _command_topic = main_topic;
    _command_topic.append("/command");   // topic for receiving commands
    _log_topic = main_topic;
    _log_topic.append("/log");           // topic wher log is sent
    _heartbeat_topic = main_topic;
    _heartbeat_topic.append("/heartbeat");           // topic where heartbeat is sent
}

void Connection::set_status_leds()
{
    if (_wifi_ok) {
        digitalWrite(_wifi_led_pin, HIGH);
    }
    else {
        digitalWrite(_wifi_led_pin, LOW);
    }

    if (_mqtt_ok) {
        digitalWrite(_mqtt_led_pin, HIGH);
    }
    else {
        digitalWrite(_mqtt_led_pin, LOW);
    }
}

void Connection::log_status()
{
    log_info("SSID: %s", _ssid.c_str());
    log_info("Passwd: %s", _passwd.c_str());
    log_info("MQTT host: %s:%d", _host.c_str(), _port);
    log_info("RSSI: %d" + WiFi.RSSI());
    log_info("IP: %s", WiFi.localIP().toString());
}

void Connection::connect(
    etl::string<64> wifi_ssid, 
    etl::string<64> wifi_passwd, 
    etl::string<64> mqtt_host, 
    etl::string<64> main_topic, 
    etl::string<512> ssl_root_ca,
    etl::string<512> ssl_cert,
    etl::string<512> ssl_key,
    uint16_t mqtt_port, 
    etl::string<64> mqtt_client_name, 
    int wifi_led_pin, 
    int mqtt_led_pin,
    bool use_ssl
    )
{
    // set private variables
    _ssid = wifi_ssid;
    _passwd = wifi_passwd;
    _host = mqtt_host;
    _port = mqtt_port;
    _client_name = mqtt_client_name;
    _main_topic = main_topic;
    _wifi_led_pin = wifi_led_pin;
    _mqtt_led_pin = mqtt_led_pin;
    _ssl_root_ca = ssl_root_ca;
    _ssl_cert = ssl_cert;
    _ssl_key = ssl_key;
    _use_ssl = use_ssl;
    
    // #ifdef ARDUINO_IOT_USE_SSL
    // #endif

    // set pin mode for LEDS
    pinMode(_wifi_led_pin, OUTPUT);
    pinMode(_mqtt_led_pin, OUTPUT);

    if (_use_ssl) {
        log_info("Setting CA cert (using SSL)");
        _wifi_secure_client.setCACert(_ssl_root_ca.c_str());
        
    // // _wifi_secure_client.setCertificate(_sslCert.c_str());
    // // _wifi_secure_client.setPrivateKey(_sslKey.c_str());
    }
    else {
        
    }
    _mqtt_client.setServer(_host.c_str(), _port );
    if (_use_ssl) {
        _mqtt_client.setClient(_wifi_secure_client);
        log_info("Connecting to MQTT using SSL");

    }
    else {
        _mqtt_client.setClient(_wifi_client);
        log_info("Connecting to MQTT without SSL");
    }

    _mqtt_client.setCallback([this](char *callbackTopic, byte *payload, unsigned int payloadLength) {
        // MQTT callback lambda function:
        number_mqtt_callbacks++;
        if ( new_mqtt_message ) {
            // ignore message while the previous message is handled
            return;
        }

        for (size_t i = 0; i < payloadLength; i++ ) {
            received_mqtt_message.push_back( (char)payload[i] );
        }
        received_mqtt_topic.assign(callbackTopic);
        new_mqtt_message = true;
    });
    
    // set all topics
    set_mqtt_main_topic(_main_topic);
    log_info("Setting MQTT main topic to: %s", _main_topic.c_str());
    
    wifi_mqtt_connect();

    // _status_interval_timer.set(30, "minutes");
}

void Connection::wifi_mqtt_connect() {
    // initalization function for establishing wifi connection
    log_info("Connecting to %s", _ssid.c_str() );
    WiFi.mode(WIFI_STA);
    
    // setup Wifi events
    WiFi.onEvent(WiFiStationWifiReady, ARDUINO_EVENT_WIFI_READY);
    WiFi.onEvent(WiFiStationWifiScanDone, ARDUINO_EVENT_WIFI_SCAN_DONE);
    WiFi.onEvent(WiFiStationStaStart, ARDUINO_EVENT_WIFI_STA_START);
    WiFi.onEvent(WiFiStationStaStop, ARDUINO_EVENT_WIFI_STA_STOP);
    WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(WiFiStationAuthmodeChange, ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE);
    WiFi.onEvent(WiFiStationGotIp, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationGotIp6, ARDUINO_EVENT_WIFI_STA_GOT_IP6);
    WiFi.onEvent(WiFiStationLostIp, ARDUINO_EVENT_WIFI_STA_LOST_IP);
    WiFi.onEvent(WiFiApStart, ARDUINO_EVENT_WIFI_AP_START);
    WiFi.onEvent(WiFiApStop, ARDUINO_EVENT_WIFI_AP_STOP);
    WiFi.onEvent(WiFiApStaConnected, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    WiFi.onEvent(WiFiApStaDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    WiFi.onEvent(WiFiApStaIpasSigned, ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
    WiFi.onEvent(WiFiApProbeEwqRecved, ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED);
    WiFi.onEvent(WiFiApGotIp6, ARDUINO_EVENT_WIFI_AP_GOT_IP6);
    WiFi.onEvent(WiFiFtmReport, ARDUINO_EVENT_WIFI_FTM_REPORT);


    WiFi.begin(_ssid.c_str(), _passwd.c_str());
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED) { 
        Serial.print(".");
        digitalWrite(_wifi_led_pin, HIGH);
        delay(200);
        digitalWrite(_wifi_led_pin, LOW);
        delay(800);
        tries++;
        if (tries > 1000) {
            log_critical("Cannot connect. Rebooting in 5 seconds...");
            delay(5000);
            ESP.restart();
        }
        log_info("Connected to WiFi after %d tries", tries);
        log_info("Time is: %s", get_time_string().c_str());
        }
    _wifi_ok = true;
    set_status_leds();

    log_info("Wifi connected. IP: %s mac: %s", WiFi.localIP().toString(), WiFi.macAddress().c_str());

    delay(2000); // letting wifi connection stabilize before connecting to MQTT - bugfix?

    _mqtt_client.setBufferSize(4096); // overrides MQTT_MAX_PACKET_SIZE in PubSubClient.h
    _mqtt_client.connect(_client_name.c_str() );
    _mqtt_client.subscribe(_command_topic.c_str() );
    log_info("Connected to broker as %s", _client_name.c_str());

    // sync ESP clock with NTP server
    //        GMT+1  Daylight saving
    configTime(3600, 3600, "0.no.pool.ntp.org", "1.no.pool.ntp.org", "2.no.pool.ntp.org");
}

void Connection::subscribe_mqtt_topic(etl::string<64> topic)
{
    _mqtt_client.subscribe(topic.c_str());
    log_info("Subscribing to topic %s", topic.c_str());
}

bool Connection::is_connected() {
    if (_wifi_ok && _mqtt_ok ) {
        return true;
    }
    return false;
}

void Connection::maintain()
{
    loop_mqtt();

    // check mqtt connection
    if ( ! _mqtt_client.state() == 0 ) {
        log_info("MQTT disconnected in maintain()");
        _mqtt_ok = false;
        set_status_leds();
        log_error("MQTT connection failed with code %d. Reconnecting.", _mqtt_client.state() );
        Connection::wifi_mqtt_connect();
    }
    else
    {
        _mqtt_ok = true;
    }

    // check wifi connection
    if ( WiFi.status() != WL_CONNECTED ) {
        log_info("Wifi not connected in maintain()");
        _wifi_ok = false;
        set_status_leds();
        log_error("Wifi not connected");
        log_error("Reconnecting");
        Connection::wifi_mqtt_connect();
    }
    else
    {
        _wifi_ok = true;
    }
    set_status_leds();

    if ( new_mqtt_message) {
        // check if any callbacks happened while parsing last message
        if ( ! (number_mqtt_callbacks == _last_number_of_callbacks + 1) ) {
            log_warning("%d mqtt callbacks ignored", number_mqtt_callbacks - _last_number_of_callbacks);
        }

        log_info("MQTT message %d (%d bytes) on %s", number_mqtt_callbacks, received_mqtt_message.size(), received_mqtt_topic.c_str());
        log_debug("Received data: %s", received_mqtt_message.c_str());

        // handle commands from MQTT
        if (received_mqtt_topic == _command_topic) {
            // message is a command
            cmd.parse(received_mqtt_message);
        }

        // Handle actions
        // Run command corresponding to the action topic
        for (size_t i = 0; i < _action_list.size(); i++) {
            if (received_mqtt_topic == _action_list[i].topic) {
                log_debug("Running action for topic %s", received_mqtt_topic.c_str());
                _action_list[i].function(received_mqtt_message);
            }
        }



        // clear variables and get ready for next message
        _last_number_of_callbacks = number_mqtt_callbacks;
        received_mqtt_message.clear();
        received_mqtt_topic.clear();
        new_mqtt_message = false;
    }

    // send heartbeat if it is time
    if ( (millis() - _last_heartbeat_millis) > HEARTBEAT_INTERVAL_MS ) {
        etl::string<32> heartbeat_string;
        etl::to_string(millis(), heartbeat_string);
        publish(_heartbeat_topic, heartbeat_string);
        _last_heartbeat_millis = millis();
    }
}

void Connection::loop_mqtt() {
    _mqtt_client.loop();
}

PubSubClient Connection::get_mqtt_client()
{
    return(_mqtt_client);
}

int Connection::publish(etl::string<128> topic, etl::string<256> message)
{
    digitalWrite(_mqtt_led_pin, LOW);
    if (_mqtt_client.publish(topic.c_str(), message.c_str()) ) {
        _mqtt_ok = true;
        set_status_leds();
        return(0);
    }
    else {
        // something went wrong with mqtt publishing
        _mqtt_ok = false;
        set_status_leds();
        return(1);
    }
    
}

void Connection::publish_log(etl::string<256> log_message) {
    publish(_log_topic, log_message);
}

etl::string<64> Connection::get_time_string() {
    struct tm timeinfo;
    // Get the local time, with a 1 second timeout for initial sync
    if (!getLocalTime(&timeinfo, 1000)) { 
        log_error("Failed to obtain time");
    }
    char timeString[100]; // Buffer to hold the formatted time string
    // Format the time using strftime
    strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    return(timeString);
    
}

void Connection::register_action(etl::string<64> topic, std::function<void(etl::string<16>)> func) {
    // register an action by topic. When a message is received on this topic, 
    // the new_action flag is set to true, and the contents of the MQTT action
    // message is stored on action_string for parsing at next tick.

    Action action;
    // action.index = _action_list.size();
    action.topic = topic;
    action.function = func;
    // action.new_action = false;
    _action_list.push_back(action);
}

void WiFiStationWifiReady(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_READY");
}

void WiFiStationWifiScanDone(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_SCAN_DONE");
}

void WiFiStationStaStart(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_START");
}

void WiFiStationStaStop(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_START");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_CONNECTED");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_DISCONNECTED");
}

void WiFiStationAuthmodeChange(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AUTH_MODE_CHANGED");
}

void WiFiStationGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_GOT_IP");
}

void WiFiStationGotIp6(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_GOT_IP_6");
}

void WiFiStationLostIp(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_STA_LOST_IP");
}

void WiFiApStart(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_START");
}

void WiFiApStop(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_STOP");
}

void WiFiApStaConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_STACONNECTED");
}

void WiFiApStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_STADISCONNECTED");
}

void WiFiApStaIpasSigned(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED");
}

void WiFiApProbeEwqRecved(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED");
}

void WiFiApGotIp6(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_AP_GOT_IP6");
}

void WiFiFtmReport(WiFiEvent_t event, WiFiEventInfo_t info) {
    log_debug("ARDUINO_EVENT_WIFI_FTM_REPORT");
}