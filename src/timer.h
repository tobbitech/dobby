#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer
{
    public:
        Timer();
        Timer(float time, String unit = "seconds", bool autostart = true);
        void set(float time, String unit = "seconds");
        void start();
        // void stop();
        void reset();
        bool is_done();
        unsigned long remaining();
        String get_set_time();

    private:
        float _time;
        String _unit;
        unsigned long _start_time_ms;
        unsigned long _wait_time_ms;
        // unsigned long current_time_ms;

};

#endif