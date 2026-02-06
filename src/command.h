#pragma once
#include<Arduino.h>
#include<etl/string.h>
#include<etl/vector.h>
#include<etl/to_arithmetic.h>
#include <ArduinoOTA.h>
#include <logging.h>


#define NO_COMMANDS 10

#define ETLSTR etl::string<32>

class Command {
    public:
        Command(uint16_t command_id, void (*cmd_func_ptr)(etl::string<32> arg_string) );
        Command();
        uint16_t get_cmd_id();
        void run(etl::string<32> arg_string);

    private:
        void (*_cmd_func_ptr)(etl::string<32> arg_string);
        uint16_t _command_id;

};

struct CommandArgs {
    uint16_t command_id;
    uint8_t n_args;
    etl::vector<ETLSTR, 5> argv;
};

class CommandParser {

    public:
        CommandParser();
        void tick();
        void parse(etl::string<32>);
        void add(uint16_t command_id, void (*cmd_func_ptr)(etl::string<32> arg_string));

    private:
        Command _cmd_list[NO_COMMANDS];
        etl::string<32> _partial_cmd_from_serial;
};


// namespace cmd_func {
    void reboot(etl::string<32> arg_string);
    void status(etl::string<32> arg_string);
    void enable_ota(etl::string<32> arg_string);
// };
