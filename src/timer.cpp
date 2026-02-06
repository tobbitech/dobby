#include "timer.h"
#include <Arduino.h>

// Default constructor
Timer::Timer() {

}

Timer::Timer(float time, String unit, bool autostart) 
{   
    Timer::set(time, unit);
        
    if (autostart) {
        Timer::start();
    }
}

void Timer::set(float time, String unit) 
{
    if (
        unit.equalsIgnoreCase("millisecond") ||
        unit.equalsIgnoreCase("milliseconds") ||
        unit.equalsIgnoreCase("millisec") ||
        unit.equalsIgnoreCase("millisecs") ||
        unit.equals("ms")
    ) { _wait_time_ms = time; }
    else if (
        unit.equalsIgnoreCase("second") ||
        unit.equalsIgnoreCase("seconds") ||
        unit.equalsIgnoreCase("sec") ||
        unit.equalsIgnoreCase("secs") ||
        unit.equals("s")
    ) { _wait_time_ms = time * 1000; }
    else if (
        unit.equalsIgnoreCase("minute") ||
        unit.equalsIgnoreCase("minutes") ||
        unit.equalsIgnoreCase("min") ||
        unit.equalsIgnoreCase("mins") ||
        unit.equals("m")
    ) { _wait_time_ms = time * 1000 * 60; }

    Timer::start();
}


void Timer::start() 
{
    _start_time_ms = millis();
    Serial.print("Timer started at ");
    Serial.println(_start_time_ms);
}

void Timer::reset()
{
    _start_time_ms = millis();
}

bool Timer::is_done()
{
    if (millis() > (_start_time_ms + _wait_time_ms)) {
        Timer::reset();
        return(true);
    }
    return(false);
}

unsigned long Timer::remaining()
{
    return((_start_time_ms + _wait_time_ms) - millis());
}

String Timer::get_set_time() {
    int total_seconds = _wait_time_ms / 1000;
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = (total_seconds % 3600) % 60;

    String time = "";
    if (hours > 0) { time += String(hours) + "h "; }
    if (minutes > 0) { time += String(minutes) + "m "; }
    time += String(seconds) + "s";

    return(time);
}
