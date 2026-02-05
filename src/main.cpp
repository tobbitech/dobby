#include <Arduino.h>
#include "command.h"

CommandParser cmd;

void setup() {
  Serial.begin(112500);
  Serial.println("Hi there!");


  cmd.add(1, reboot);
  cmd.add(2, status);
  cmd.add(3, enable_ota);
}



void loop() {
  cmd.tick();
  
}