#pragma once

#include <Arduino.h>

class Status {
public:
  void initializing()
  {
    Serial.println("Initializing...");

  }
  void initialized()
  {
    Serial.printf("Initialised");
  }
  void error()
  {
    Serial.println("Erreur");
  }

};

