#include "command.h"


Command::Command() {
    _command_id = 0;
    _cmd_func_ptr = nullptr;
    _help_text.clear();
}

Command::Command(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args), etl::string<64> help_text) {
    _cmd_func_ptr = cmd_func_ptr;
    _command_id = command_id;
    _help_text.assign(help_text);
}

uint16_t Command::get_cmd_id() {
    return(_command_id);
}

void Command::run(CommandArgs args) {
    _cmd_func_ptr(args);
}

etl::string_view Command::help() {
    return _help_text;
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

void CommandParser::log_cmd_help_text(uint16_t command_id) {
    log_response("--- Available commands on device ---");
    for (uint16_t i = 0; i < NO_COMMANDS; i++) {
        if (_cmd_list[i].get_cmd_id() != 0) {
            etl::string<64> help_text(_cmd_list[i].help());
            log_response("%d: %s", i, help_text.c_str());
        }
    }
}

void CommandParser::add(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args), etl::string<64> help_text) {
    auto command = Command(command_id, cmd_func_ptr, help_text);
    
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




