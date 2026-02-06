#include "command.h"


Command::Command() {
    // empty constructor
}

Command::Command(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args) ) {
    _cmd_func_ptr = cmd_func_ptr;
    _command_id = command_id;
}

uint16_t Command::get_cmd_id() {
    return(_command_id);
}

void Command::run(CommandArgs args) {
    _cmd_func_ptr(args);
}

CommandParser::CommandParser() {
    _partial_cmd_from_serial.clear();
}

void CommandParser::parse(ETLSTR cmd_string) {
    // remove whitespace from end of line
    // Iterate from the end and check if the character is in the set of characters to remove
    while (!cmd_string.empty() && 
        (cmd_string.back() == '\n' || cmd_string.back() == '\r' || cmd_string.back() == ' '))
    {
        cmd_string.pop_back(); // Remove the last character
    }

    // log_info("Recevied: %s", cmd_string.c_str() );

    CommandArgs args;
    // casts output of find to int to get negative value when no comma is found
    int comma_pos = cmd_string.find(",");

    // check if cmd is just command id or comma delimted list:
    if ( comma_pos > 0 ) {
        // comma delimited list
        etl::vector<etl::string_view, 6> argv;

        bool all_views_found = etl::get_token_list(cmd_string, argv, ",", false, 6);
        if (!all_views_found) {
            log_warning("Got more than 5 arguments. Omitting the excess arguments.");
        }
        
        args.command_id = etl::to_arithmetic<uint16_t>(argv[0]);
        args.n_args = argv.size() - 1; // subtract command ID
        log_info("Received command ID: %d with %d arguments", args.command_id, args.n_args);

        for (size_t i = 1; i < argv.size(); i++) {
            args.argv.push_back(etl::string<32>(argv[i]));
            log_debug("%d: %s", i, args.argv[i-1].c_str());
        }
    }
    else {
        args.n_args = 0;
        args.command_id = etl::to_arithmetic<uint16_t>(cmd_string);
    }

    if ( args.command_id == 0 ) {
        // error parsing command
        log_error("Cannot parse: %s", cmd_string.c_str());
        return;
    }
    
    if (command_id_exists(args.command_id)) {
        _cmd_list[args.command_id].run(args);
    }
}


 bool CommandParser::command_id_exists(uint16_t command_id) {
    for (size_t i = 0; i < NO_COMMANDS; i++) {
        if (_cmd_list[i].get_cmd_id() == command_id) {
            return(true);
        }
    }
    log_warning("Command ID %d not found", command_id);
    return(false);
 }

// void CommandParser::run_cmd(CommandArgs args) {
    
// }

void CommandParser::add(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args)) {
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



