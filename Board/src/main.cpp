#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#include "QuadFlyController.h"

QuadFlyController FC;
FCQuadState State;

float g_TotalTime = 0.0f; // in s
float g_DeltaTime = 0.0f; // in s

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

const float k_AccXOff = 0.04f;
const float k_AccYOff = 0.03f;
const float k_AccZOff = -0.01f;

const float k_GyroXOff = -0.32f;
const float k_GyroYOff = -0.475f;
const float k_GyroZOff = -0.05f;

void InitBLE();
void InitIMU();

bool GetRawAccel(float& x, float& y, float& z);
bool GetRawGyro(float& x, float& y, float& z);

void setup() 
{
  Serial.begin(9600);
  while (!Serial) {}
  //InitBLE();
  InitIMU();
}

void loop() 
{
  unsigned long startTime = millis();

  bool IMUValid = true;
  float rawPitch = 0.0f;
  float rawRoll = 0.0f;

  float ax, ay, az;
  if(GetRawAccel(ax, ay, az))
  {
    float accMagnitude = sqrt((ax*ax) + (ay*ay) + (az*az));
    rawPitch = atan2((ax / accMagnitude) , (az / accMagnitude)) * RAD_TO_DEG;
    rawRoll = atan2((-ay / accMagnitude) , (az / accMagnitude)) * RAD_TO_DEG;   
  }
  else
  {
    Serial.println("ERROR READING ACCEL"); // Atm, we should stop the drone if we get this
  }
  

  float wx, wy, wz;
  if(!GetRawGyro(wx, wy, wz))
  {
    Serial.println("ERROR READING GYR"); // Atm, we should stop the drone if we get this
  }

  static float s_AcumYaw = 0.0f;
  static float s_AcumPitch = 0.0f;
  static float s_AcumRoll = 0.0f;
  static bool s_FirstTime = true;

  if(s_FirstTime)
  {
    s_FirstTime =  false;
    s_AcumYaw = 0.0f;
    s_AcumPitch = rawPitch;
    s_AcumRoll = rawRoll;
  }
  else
  {
    s_AcumYaw += wz * g_DeltaTime;
    s_AcumPitch += wy * g_DeltaTime;
    s_AcumRoll += wx * g_DeltaTime;
  }
  
  // Transfer angle as we have yawed:
  s_AcumPitch -= s_AcumRoll * sin((wz * g_DeltaTime) * PI / 180.0f);
  s_AcumRoll += s_AcumPitch * sin((wz * g_DeltaTime) * PI / 180.0f);

  // Combine raw accel and gyro, this adds noise but removes gyro drift over time:
  s_AcumPitch = s_AcumPitch * 0.9f + rawPitch * 0.1f;
  s_AcumRoll = s_AcumRoll * 0.9f + rawRoll * 0.1f;

  // Debug:
  Serial.print(s_AcumPitch); Serial.print(",");
  Serial.print(s_AcumYaw);Serial.print(",");
  Serial.println(s_AcumRoll);

  delay(10); // Workaround as we are sampling too fast the IMU..

  // End of the iteration, compute delta, acum total:
  g_DeltaTime = ((float)startTime - (float)millis()) / 1000.0f;
  g_TotalTime += g_DeltaTime;

#if 0
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
#endif
}

void InitBLE()
{
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

void InitIMU()
{
  if(!IMU.begin())
  {
    Serial.println("Failed to init the IMU");
    while(1){};
  }
  
  
  // TO-DO: automate calibration process:

  // Acceleration calibration:
#if 0
  delay(2000);
  const int numSamples = 1000;
  float totalX = 0.0f;
  float totalY = 0.0f;
  float totalZ = 0.0f;
  for(int i=0; i<numSamples; ++i)
  {
    float x,y,z;
    while(!IMU.accelerationAvailable())
    {
      delay(5);
    }
    if(IMU.readAcceleration(x, y, z))
    {
      totalX += x;
      totalY += y;
      totalZ += z;
    }
  }
  totalX = totalX / (float)numSamples;
  totalY = totalY / (float)numSamples;
  totalZ = totalZ / (float)numSamples;

  Serial.print(totalX);
  Serial.print('\t');
  Serial.print(totalY);
  Serial.print('\t');
  Serial.println(totalZ);
#endif

  // Gyro calibration:
#if 0
  delay(2000);
  const int numSamples = 1000;
  float totalX = 0.0f;
  float totalY = 0.0f;
  float totalZ = 0.0f;
  for(int i=0; i<numSamples; ++i)
  {
    float x,y,z;
    while(!IMU.gyroscopeAvailable())
    {
      delay(5);
    }
    if(IMU.readGyroscope(x, y, z))
    {
      totalX += x;
      totalY += y;
      totalZ += z;
    }
  }
  totalX = totalX / (float)numSamples;
  totalY = totalY / (float)numSamples;
  totalZ = totalZ / (float)numSamples;

  Serial.print(totalX);
  Serial.print('\t');
  Serial.print(totalY);
  Serial.print('\t');
  Serial.println(totalZ);
#endif
}

bool GetRawAccel(float& x, float& y, float& z)
{
  if(!IMU.accelerationAvailable() || !IMU.readAcceleration(x, y, z))
  {
    return false;
  }
  x += k_AccXOff;
  y += k_AccYOff;
  z += k_AccZOff;
  return true;
}

bool GetRawGyro(float& x, float& y, float& z)
{
  if(!IMU.gyroscopeAvailable() || !IMU.readGyroscope(x, y, z))
  {
    return false;
  }
  x += k_GyroXOff;
  y += k_GyroYOff;
  z += k_GyroZOff;
  return true;
}