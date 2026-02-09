#include "timer.h"

// Default constructor
Timer::Timer() {

}

Timer::Timer(float time, char unit, bool autostart) 
{   
    Timer::set(time, unit);
        
    if (autostart) {
        Timer::start();
    }
}

void Timer::set(float time, char unit) 
{
    if ( unit == 'm' ) { _wait_time_ms = time; }
    else if ( unit == 's' ) { _wait_time_ms = time * 1000; }
    else if ( unit == 'M' ) { _wait_time_ms = time * 1000 * 60; }

    Timer::start();
}

void Timer::start() 
{
    _start_time_ms = millis();
    log_debug("Timer started at %lu", _start_time_ms);
}

void Timer::reset()
{
    _start_time_ms = millis();
    log_debug("Timer reset at %lu", _start_time_ms);
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

// etl::string<32> Timer::get_set_time() {
//     int total_seconds = _wait_time_ms / 1000;
//     int hours = total_seconds / 3600;
//     int minutes = (total_seconds % 3600) / 60;
//     int seconds = (total_seconds % 3600) % 60;

//     etl::string<32> time = "";
//     if (hours > 0) { 
//         etl::to_string(hours, time, etl::format_spec(), true);
//         time.append("h ");
//     }
//     if (minutes > 0) {
//         etl::to_string(minutes, time, etl::format_spec(), true);
//         time.append("m ");
//     }
//     if (seconds > 0) {
//         etl::to_string(seconds, time, etl::format_spec(), true);
//         time.append("s");
//     }
//     return(time);
// }
