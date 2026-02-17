#pragma once
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "command.h"
#include "mqttConnection.h"
#include "logging.h"
#include <time.h>

extern CommandParser cmd;
extern Connection conn;

namespace CMD {

    void list_commands(CommandArgs args) {
        cmd.log_cmd_help_text();
    }

    void reboot(CommandArgs args) {
        ESP.restart();
    }

    void status(CommandArgs args) {
        Serial.println("Printing status...  (not implemented yet)");
        // status message:
        // Memory
        // Time + uptime
        // mqtt messages sent
        // mqtt messages received
        log_response("Device name: %s", conn.get_mqtt_client_name() );
        log_response("Connected to %s with RSSI %ddB", WiFi.SSID().c_str(), WiFi.RSSI() );
        log_response("Using %.1f%% of memory", (float(ESP.getFreeHeap())/float(ESP.getHeapSize()))*100 );

        uint32_t uptime_ms = (millis() / 1000);
        uint16_t days = uptime_ms / (24*60*60);
        uptime_ms %= (24*60*60);
        uint8_t hours = uptime_ms / (60*60);
        uptime_ms %= (60*60);
        uint8_t minutes = uptime_ms / 60;
        uint8_t seconds = uptime_ms % 60;

        log_response("Uptime: %dd %dh %dm %ds", days, hours, minutes, seconds);
        log_response("Time is: %s", conn.get_time_string().c_str());

    }

    void enable_ota(CommandArgs args) {
        log_response("Enabling OTA");

        // Stores OTA progress to avoid spamming logs with same percentage
        unsigned int last_progress = 0 ;
        
        ArduinoOTA
            .onStart([]() {
                etl::string<16> type;
                if (ArduinoOTA.getCommand() == U_FLASH)
                    type = "sketch";
                else // U_SPIFFS
                    type = "filesystem";

                // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                log_response("Start updating %s", type);
            })
            .onEnd([]() {
                log_response("Data transmission completed");
            })
            .onProgress([&last_progress](unsigned int progress, unsigned int total) {
                unsigned int progress_pct = (progress / (total / 100));
                if (progress_pct != last_progress ) {
                    log_response("Progress: %u%%\r", progress_pct);
                    last_progress = progress_pct;
                }
            })
            .onError([](ota_error_t error) {
                log_error("OTA Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) log_error("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) log_error("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) log_error("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) log_error("Receive Failed");
                else if (error == OTA_END_ERROR) log_error("End Failed");
            });

        ArduinoOTA.begin();

        for (int seconds_passed = 0; seconds_passed < 60; seconds_passed++) {
            ArduinoOTA.handle();
            if (seconds_passed % 10 == 0) {
                log_info("OTA active for %d seconds", seconds_passed);
            }
            delay(1000);
        }
        ArduinoOTA.end();
        log_error("OTA sessions completed, no new firmware received.");
    }

    void set_wifi(CommandArgs args) {
        // Sets a wifi and SSID and password
        // arg 1: SSID
        // arg 2: password

        log_warning("Settint wifi not implemented yet");
    }


    void set_log_level(CommandArgs args ) {
        if (! check_args(args, 1)) { return; }

        if (args.argv[0] == "DEBUG") {
            set_log_level(log_severity::DEBUG);
        } else if (args.argv[0] == "INFO") {
            set_log_level(log_severity::INFO);
        } else if (args.argv[0] == "WARNING") {
            set_log_level(log_severity::WARNING);
        } else if (args.argv[0] == "ERROR") {
            set_log_level(log_severity::ERROR);
        } else if (args.argv[0] == "CRITICAL") {
            set_log_level(log_severity::CRITICAL);
        } else if (args.argv[0] == "RESPONSE") {
            set_log_level(log_severity::RESPONSE);
        }
    }

    void log_ip(CommandArgs args) {
        log_response("IP: %s", WiFi.localIP().toString());
    }

    void log_mac(CommandArgs args) {
        log_response("MAC: %s", WiFi.macAddress().c_str());
    }
}