#include <Arduino.h>
#include <ArduinoBLE.h>

#include "QuadFlyController.h"

BLEService g_TestService("180F");

QuadFlyController FC;
FCQuadState State;

float totalTime = 0.0f; // in ms
float deltaTime = 0.0f; // in ms

const int k_PinMotor1 = 5;
const int k_PinMotor2 = 4;
const int k_PinMotor3 = 3;
const int k_PinMotor4 = 2;

void setup() 
{
  delay(5000);

  analogWrite(k_PinMotor1, 5);
  analogWrite(k_PinMotor2, 5);
  analogWrite(k_PinMotor3, 5);
  analogWrite(k_PinMotor4, 5);

  delay(10000);

  analogWrite(k_PinMotor1, 0);
  analogWrite(k_PinMotor2, 0);
  analogWrite(k_PinMotor3, 0);
  analogWrite(k_PinMotor4, 0);
}

void loop() 
{
  unsigned long startTime = millis();

  State.DeltaTime = deltaTime;
  State.Time = totalTime;

  State.Height = 0.0f;
  State.Pitch = 0.0f;
  State.Yaw = 0.0f;
  State.Roll = 0.0f;

  FCCommands commands = FC.Iterate(State);

  deltaTime = (float)startTime - (float)millis();
  totalTime += deltaTime;
}