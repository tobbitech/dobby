#include <logging.h>
#include <mqttConnection.h>

extern Connection conn;

void set_log_level(log_severity new_log_level) {
    log_level = new_log_level;
}

void log(etl::string<LOG_STRING_LENGTH> message, log_severity severity, bool only_serial, bool store_in_nvm) {
    // Sends message to mqtt and serial (if not flag is set to false)
    // can also store log message in nvm log if flag is set

    // Only log the message if severity is above or equal to log_level
    if (severity < log_level) {
        return;
    }

    uint32_t ms = millis();
    float seconds = (float)ms/1000.0;

    etl::to_string(seconds, timestamp, etl::format_spec().precision(1), false);

    switch (severity) {
        case log_severity::DEBUG: {
            modified_log_message.assign("[DEBUG:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
        case log_severity::INFO: {
            modified_log_message.assign("[INFO:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
        case log_severity::WARNING: {
            modified_log_message.assign("[WARNING:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
        case log_severity::ERROR: {
            modified_log_message.assign("[ERROR:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
        case log_severity::CRITICAL: {
            modified_log_message.assign("[CRITICAL:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
        case log_severity::RESPONSE: {
            modified_log_message.assign("[RESPONSE:");
            modified_log_message.append(timestamp);
            modified_log_message.append("] ");
        } break;
    }

    modified_log_message.append(message);

    Serial.println(modified_log_message.c_str());
    
    if (!only_serial && conn.is_connected() ) {
        conn.publish_log(modified_log_message );
        conn.loop_mqtt();
    }

    if (store_in_nvm) {
        // TODO: Store in NVM
    }

    modified_log_message.clear();
    timestamp.clear();
}

void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::DEBUG, false, false);
}

void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::INFO, false, false);
}

void log_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::WARNING, false, false);
}

void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::ERROR, false, false);
}

void log_critical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::CRITICAL, false, false);
}

void log_response(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_STRING_LENGTH, format, args);
    va_end(args);

    log(buffer, log_severity::RESPONSE, false, false);
}