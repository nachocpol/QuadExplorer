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
BLEService g_CommandsService("1101");
BLEIntCharacteristic g_ThrottleCharacteristic("2202", BLERead | BLEWriteWithoutResponse);
BLEIntCharacteristic g_YawCharacteristic("2203", BLERead | BLEWriteWithoutResponse);
BLEIntCharacteristic g_PitchCharacteristic("2204", BLERead | BLEWriteWithoutResponse);
BLEIntCharacteristic g_RollCharacteristic("2205", BLERead | BLEWriteWithoutResponse);

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

  // Ensure 0 initialized:
  g_ThrottleCharacteristic.setValue(0);
  g_YawCharacteristic.setValue(0);
  g_PitchCharacteristic.setValue(0);
  g_RollCharacteristic.setValue(0);

  // Advertise commands service and characteristics:
  BLE.setLocalName("QuadExplorer");
  BLE.setAdvertisedService(g_CommandsService);
  g_CommandsService.addCharacteristic(g_ThrottleCharacteristic);
  g_CommandsService.addCharacteristic(g_YawCharacteristic);
  g_CommandsService.addCharacteristic(g_PitchCharacteristic);
  g_CommandsService.addCharacteristic(g_RollCharacteristic);
  BLE.addService(g_CommandsService);
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
    int32_t throttle = 0;
    int32_t yaw = 0;
    int32_t pitch = 0;
    int32_t roll = 0;
    
    g_ThrottleCharacteristic.readValue(throttle);
    g_YawCharacteristic.readValue(yaw);
    g_PitchCharacteristic.readValue(pitch);
    g_RollCharacteristic.readValue(roll);

    Serial.print(throttle); Serial.print(",");
    Serial.print(yaw); Serial.print(",");
    Serial.print(pitch); Serial.print(",");
    Serial.println(roll);
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