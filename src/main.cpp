#include <Arduino.h>
#include "command.h"
#include "logging.h"
#include "command_funcs.h"

CommandParser cmd;

void setup() {

  set_log_level(log_severity::DEBUG);

  Serial.begin(112500);
  Serial.println("Hi there!");

  cmd.add(1, list_commands, "Lists available commands");
  cmd.add(2, reboot, "Reboots device");
  cmd.add(3, status, "Shows status of device");
  cmd.add(4, enable_ota, "Sets device in OTA mode for 60 seconds");
}



void loop() {
  cmd.tick();
  
}