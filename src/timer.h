#pragma once

#include <Arduino.h>
#include "logging.h"

class Timer
{
    public:
        Timer();
        Timer(float time, char unit = 's', bool autostart = true);
        void set(float time, char unit = 's');
        void start();
        void reset();
        bool is_done();
        unsigned long remaining();
        etl::string<32> get_set_time();

    private:
        float _time;
        char _unit;
        unsigned long _start_time_ms;
        unsigned long _wait_time_ms;
};