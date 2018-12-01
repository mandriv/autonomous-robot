#ifndef ULTRASOUND_SENSOR_CONTROLLER_H
#define ULTRASOUND_SENSOR_CONTROLLER_H
#include <Arduino.h>

long microsecondsToCentimeters(long microseconds);

unsigned int readPing(int triggerPin, int echoPin);

#endif
