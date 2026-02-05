#include "command.h"


Command::Command() {
    // empty constructor
}

Command::Command(uint16_t command_id, void (*cmd_func_ptr)(etl::string<32> arg_string) ) {
    _cmd_func_ptr = cmd_func_ptr;
    _command_id = command_id;
}

uint16_t Command::get_cmd_id() {
    return(_command_id);
}

void Command::run(etl::string<32> arg_string) {
    _cmd_func_ptr(arg_string);
}


CommandParser::CommandParser() {
    // _partial_cmd_from_serial.clear();
}

void CommandParser::parse(etl::string<32> cmd_string) {
    Serial.print("Recevied: ");
    Serial.println(cmd_string.c_str());
    size_t comma_i = cmd_string.find(',');
    etl::string<6> cmd_id_string;

    Serial.print("Comma found as letter: ");
    Serial.println(comma_i);

    // for (size_t i = 0; i < comma_i; i++ ) {
    //     cmd_id_string[i] = cmd_string[i];
    // }

    // uint16_t cmd_id = etl::to_arithmetic<uint16_t>(cmd_id_string, cmd_id_string.size());

    uint16_t cmd_id = etl::to_arithmetic<uint16_t>(cmd_string, cmd_string.size());

    Serial.print("Conversion to int: ");
    Serial.println(cmd_id);

    // Serial.print("Received command id: ");
    // Serial.println(cmd_id);
}

void CommandParser::add(uint16_t command_id, void (*cmd_func_ptr)(etl::string<32> arg_string)) {
    auto command = Command(command_id, cmd_func_ptr);
    
    _cmd_list[command_id] = command;
}

void CommandParser::tick() {
    // handle serial
    auto c = Serial.read();
    if ( c == '\n') {
        parse(_partial_cmd_from_serial);
        _partial_cmd_from_serial.clear();
    }
    else if (c < 0) {
        // nothinh received, do nothing
    }
    else {
        _partial_cmd_from_serial.push_back(c);
    }

}
// using namespace cmd_func;

void reboot(etl::string<32> arg_string) {
    ESP.restart();
}

void status(etl::string<32> arg_string) {
    Serial.println("Printing status...  (not implemented yet)");
}

void enable_ota(etl::string<32> arg_string) {
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



