#pragma once
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <command.h>

extern CommandParser cmd;

void list_commands(CommandArgs args) {
    cmd.log_cmd_help_text();
}

void reboot(CommandArgs args) {
    ESP.restart();
}

void status(CommandArgs args) {
    Serial.println("Printing status...  (not implemented yet)");
}

void enable_ota(CommandArgs args) {
    Serial.println("Enabling OTA");

    ArduinoOTA
        .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        })
        .onEnd([]() {
        Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

    ArduinoOTA.begin();

    for (int seconds_passed = 0; seconds_passed < 60; seconds_passed++) {
        ArduinoOTA.handle();
        if (seconds_passed % 10 == 0) {
            Serial.println(seconds_passed);
        }
        delay(1000);
    }
    ArduinoOTA.end();
    Serial.println("OTA sessions completed, no new firmware received.");
}

void set_wifi(CommandArgs args) {
    // Sets a wifi and SSID and password
    // arg 1: SSID
    // arg 2: password

    log_warning("Settint wifi not implemented yet");
}


void log_ip(CommandArgs args) {

}

void log_mac(CommandArgs args) {

}