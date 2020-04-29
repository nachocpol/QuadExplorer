#include <Arduino.h>
#include <ArduinoBLE.h>

#include "QuadFlyController.h"

QuadFlyController FC;
FCQuadState State;

float totalTime = 0.0f; // in ms
float deltaTime = 0.0f; // in ms

const int k_PinMotor1 = 5;
const int k_PinMotor2 = 4;
const int k_PinMotor3 = 3;
const int k_PinMotor4 = 2;

BLEDevice g_CentralDevice;

void setup() 
{
  Serial.begin(9600);
  while (!Serial) {}

  if(!BLE.begin())
  {
    Serial.println("Failed to begin BLE");
    while(1);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  String bleAddress = BLE.address();
  Serial.print("Local address is : "); Serial.println(bleAddress);

  BLE.setAdvertisedServiceUuid("19B10000-E8F2-537E-4F6C-D104768A1214");
  BLE.setLocalName("QuadExplorer");
  BLE.setDeviceName("ArduinoNanoBLE33");

  BLE.advertise();

  // Wait until the central device connects:
  while(!g_CentralDevice)
  {
    g_CentralDevice = BLE.central();
    delay(10);
  }

  Serial.print("We have a central: "); Serial.println(g_CentralDevice.address());
  if(g_CentralDevice.hasLocalName())
  {
    Serial.print("Name: "); Serial.println(g_CentralDevice.localName());
  }
  if(!g_CentralDevice.discoverAttributes())
  {
    Serial.println("Failed to discover attribs");
  }

  if(!g_CentralDevice.connect())
  {
    Serial.println("Failed to connect to the central device");
    while(1);
  }
}

void loop() 
{
  // Check if still connected, this does the poll (with 0ms time out)
  if(!g_CentralDevice.connected())
  {
    Serial.println("Lost connection with central device");
    while(1);
  }
  else
  {
    Serial.println(g_CentralDevice.rssi());
  }

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