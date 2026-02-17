#pragma once
#include <Arduino.h>
#include <etl/string.h>
#include <etl/vector.h>
#include <etl/to_arithmetic.h>
#include <logging.h>


#define NO_COMMANDS 20

#define ETLSTR etl::string<128>

struct CommandArgs {
    uint16_t command_id;
    uint8_t n_args;
    etl::vector<ETLSTR, 5> argv;
};

class Command {
    public:
        Command(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args), etl::string<64> help_text );
        Command();
        uint16_t get_cmd_id();
        void run(CommandArgs args);
        etl::string_view help();

    private:
        void (*_cmd_func_ptr)(CommandArgs args);
        uint16_t _command_id;
        etl::string<64> _help_text;

};


class CommandParser {

    public:
        CommandParser();
        void tick();
        void parse(ETLSTR);
        void run_cmd(CommandArgs args);
        void add(uint16_t command_id, void (*cmd_func_ptr)(CommandArgs args), etl::string<64> help_text);
        bool command_id_exists(uint16_t command_id);
        void log_cmd_help_text(uint16_t command_id = 0);

    private:
        Command _cmd_list[NO_COMMANDS];
        ETLSTR _partial_cmd_from_serial;
};

bool check_args(CommandArgs args, uint8_t n);

// namespace cmd_func {
    // void reboot(CommandArgs args);
    // void status(CommandArgs args);
    // void enable_ota(CommandArgs args);
// };
