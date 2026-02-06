#pragma once
#include<Arduino.h>
#include<etl/string.h>
#include<etl/vector.h>
#include<etl/to_arithmetic.h>
#include <ArduinoOTA.h>
#include <logging.h>


#define NO_COMMANDS 10

#define ETLSTR etl::string<128>

struct CommandArgs {
    uint16_t command_id;
    uint8_t n_args;
    etl::vector<ETLSTR, 5> argv;
};

class Command {
    public:
        Command(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args) );
        Command();
        uint16_t get_cmd_id();
        void run(CommandArgs args);

    private:
        void (*_cmd_func_ptr)(CommandArgs args);
        uint16_t _command_id;

};


class CommandParser {

    public:
        CommandParser();
        void tick();
        void parse(ETLSTR);
        void run_cmd(CommandArgs args);
        void add(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args));
        bool command_id_exists(uint16_t command_id);

    private:
        Command _cmd_list[NO_COMMANDS];
        ETLSTR _partial_cmd_from_serial;
};


// namespace cmd_func {
    void reboot(CommandArgs args);
    void status(CommandArgs args);
    void enable_ota(CommandArgs args);
// };
