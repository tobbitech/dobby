#pragma once

#include <Arduino.h>
#include <etl/string.h>
#include <etl/to_arithmetic.h>
#include <etl/to_string.h>
#include <cstdio>
#include <cstdarg>

#define LOG_STRING_LENGTH 100

enum class log_severity : uint8_t {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    RESPONSE
};

static log_severity log_level = log_severity::INFO;

void set_log_level(log_severity new_log_level);

static etl::string<LOG_STRING_LENGTH + 15> modified_log_message; 
static etl::string<24> timestamp; 
static char buffer[LOG_STRING_LENGTH];

void log(
    etl::string<LOG_STRING_LENGTH> message, 
    log_severity severity = log_severity::INFO, 
    bool only_serial = false,
    bool store_in_nvm = false
    );

void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);
void log_critical(const char* format, ...);
void log_response(const char* format, ...);
