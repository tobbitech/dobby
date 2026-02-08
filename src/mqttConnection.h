#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include "logging.h"
#include <etl/string.h>
#include <etl/to_arithmetic.h>
#include "command.h"

// #define ARDUINO_IOT_USE_SSL
#define HEARTBEAT_INTERVAL_MS 5000

class Connection 
{
    public:
        Connection();
        void connect(            
            etl::string<64> wifi_ssid, 
            etl::string<64> wifi_passwd, 
            etl::string<64> mqtt_host, 
            etl::string<64> main_topic, 
            std::function<void(char*, uint8_t*, unsigned int)> callback,
            etl::string<512> sslRootCa = "",
            etl::string<512> sslCert = "",
            etl::string<512> sslKey = "",
            uint16_t mqtt_port = 1883, 
            etl::string<64> mqtt_client_name = "MqttClient", 
            int wifi_led_pin = 4, 
            int mqtt_led_pin = 5,
            bool use_ssl= false
        );
        void wifi_mqtt_connect();
        void maintain();
        PubSubClient get_mqtt_client();
        void log_status();
        int publish(etl::string<128> topic, etl::string<256> message);
        void publish_log(etl::string<256>);
        void set_status_leds();
        unsigned long get_timestamp();
        etl::string<64> get_timestamp_millis();
        void set_wifi_ssid(etl::string<64> ssid);
        void set_wifi_passwd(etl::string<64> passwd);
        void set_mqtt_host(etl::string<64> host);
        void set_mqtt_port(int port);
        void set_mqtt_port(etl::string<64> port);
        void set_mqtt_client_name(etl::string<64> clientName);
        void set_mqtt_main_topic(etl::string<64> mainTopic);
        void subscribe_mqtt_topic(etl::string<64> topic);
        // void set_ssl_ca(etl::string<64> ca);
        // void set_ssl_cert(etl::string<64> cert);
        // void set_ssl_key(etl::string<64> key);
        bool is_connected();
        etl::string<128> received_mqtt_topic;
        etl::string<256> received_mqtt_message; // mqtt callback stores payload in this variable
        bool new_mqtt_message;
        uint32_t number_mqtt_callbacks;
        void loop_mqtt();

    private:
        etl::string<64> _ssid;
        etl::string<64> _passwd;
        etl::string<64> _host;
        bool _use_ssl;
        uint16_t _port;
        etl::string<64> _client_name;
        etl::string<64> _main_topic;

        // #ifdef ARDUINO_IOT_USE_SSL
        // WiFiClientSecure _wifiClient = WiFiClientSecure();
        // #else
        WiFiClient _wifi_client = WiFiClient();
        WiFiClientSecure _wifi_secure_client = WiFiClientSecure();
        // #endif

        PubSubClient _mqtt_client;
        etl::string<64> _command_topic;
        etl::string<64> _log_topic;
        etl::string<64> _heartbeat_topic;
        etl::string<64> _ssl_root_ca;
        etl::string<64> _ssl_key;
        etl::string<64> _ssl_cert;
        bool _mqtt_ok;
        bool _wifi_ok;
        int _wifi_led_pin;
        int _mqtt_led_pin;
        WiFiUDP _ntp_udp;
        NTPClient _time_client(WiFiUDP);
        uint32_t _last_number_of_callbacks;
        uint32_t _last_heartbeat_millis;
};

void WiFiStationWifiReady(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationWifiScanDone(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationStaStart(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationStaStop(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationAuthmodeChange(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationGotIp(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationGotIp6(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationLostIp(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStart(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStop(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStaConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStaDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApStaIpasSigned(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApProbeEwqRecved(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiApGotIp6(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiFtmReport(WiFiEvent_t event, WiFiEventInfo_t info);