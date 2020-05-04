#include <Arduino.h>
#include <SPI.h>

#include "Smch.h"

Smch* _smch;

void setup()
{
    Serial.begin(115200);
    SPI.begin();

    static Smch smch;
    _smch = &smch;
}

void loop()
{
    _smch->task();
}