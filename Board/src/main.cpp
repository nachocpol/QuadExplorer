#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#include "QuadFlyController.h"

QuadFlyController FC;

float g_TotalTime = 0.0f; // in s
float g_DeltaTime = 0.0f; // in s

const int k_PinMotorRR = 5; // Rear_Right
const int k_PinMotorRL = 4; // Rear_Left
const int k_PinMotorFL = 3; // Front_Left
const int k_PinMotorFR = 2; // Front_Right

BLEDevice g_CentralDevice;
BLEService g_CommandsService("1101");
BLEByteCharacteristic g_StopCharacteristic("2206", BLEWrite);
BLEUnsignedLongCharacteristic g_PackedCharacteristic("2207", BLERead | BLEWrite);

const float k_AccXOff = 0.04f;
const float k_AccYOff = 0.03f;
const float k_AccZOff = -0.01f;

const float k_GyroXOff = -0.32f;
const float k_GyroYOff = -0.475f;
const float k_GyroZOff = -0.05f;

void InitBLE();
void InitIMU();
void Halt();

bool GetRawAccel(float& x, float& y, float& z);
bool GetRawGyro(float& x, float& y, float& z);

// Fills current orientation in degrees.
void GetOrientation(float& yaw, float& pitch, float& roll);

// Queries the commands from the connected controller app:
//   Throttle [0,100]
//   Yaw      [-100,100]
//   Pitch    [-100,100]
//   Roll     [-100,100]
void GetControlCommandsRaw(int32_t* throttle, int32_t* yaw, int32_t* pitch, int32_t* roll);

// Returns controls remaped (throt 0-100). Orientation in radians
void GetControlCommands(float& throttle, float& yaw, float& pitch, float& roll);

void setup() 
{
  Serial.begin(9600);
  //while (!Serial) {}

  // Test motors
#if 0
    analogWrite(k_PinMotorFL, 5);
    delay(2500);
    analogWrite(k_PinMotorFL, 0);

    analogWrite(k_PinMotorFR, 5);
    delay(2500);
    analogWrite(k_PinMotorFR, 0);

    analogWrite(k_PinMotorRR, 5);
    delay(2500);
    analogWrite(k_PinMotorRR, 0); 

    analogWrite(k_PinMotorRL, 5);
    delay(2500);
    analogWrite(k_PinMotorRL, 0);

    while(1){};
#else
  analogWrite(k_PinMotorFL, 0);
  analogWrite(k_PinMotorFR, 0);
  analogWrite(k_PinMotorRL, 0);
  analogWrite(k_PinMotorRR, 0);
#endif

  InitBLE();
  InitIMU();
}

void loop() 
{
  unsigned long startTime = millis();
  {
    // Check if still connected, this does the poll (with 0ms time out)
    if(!g_CentralDevice.connected())
    {
      Serial.println("[HALT!] Lost connection with central device");
      Halt();
    }

    // Check if controller requested emergency stop:
    byte stop = 0x0;
    g_StopCharacteristic.readValue(&stop, 1);
    if(stop == 0x1)
    {
      Serial.println("[HALT!] Controller requested STOP");
      Halt();
    }

    // Setup quad state for this iteration:
    FCQuadState curState = {};
    GetOrientation(curState.Yaw, curState.Pitch, curState.Roll);
    curState.Yaw *= DEG_TO_RAD;
    curState.Pitch *= DEG_TO_RAD;
    curState.Roll *= DEG_TO_RAD;
    curState.DeltaTime = g_DeltaTime;
    curState.Time = g_TotalTime;

    // Query commands:
    FCSetPoints setPoints;
    GetControlCommands(setPoints.Thrust, setPoints.Yaw, setPoints.Pitch, setPoints.Roll);

    // Iterate FC
    FCCommands curCommands = FC.Iterate(curState, setPoints);

    // Ug, reverse it to match sim frame of reference:
    curCommands.FrontLeftThr = curCommands.FrontLeftThr;
    curCommands.FrontRightThr = curCommands.FrontRightThr;
    curCommands.RearLeftThr = curCommands.RearLeftThr;
    curCommands.RearRightThr = curCommands.RearRightThr;

    float capThrottle = 250.0f;
    int fl = (int)constrain((250.0f * curCommands.FrontLeftThr), 0.0f, capThrottle);
    int fr = (int)constrain((250.0f * curCommands.FrontRightThr), 0.0f, capThrottle);
    int rl = (int)constrain((250.0f * curCommands.RearLeftThr), 0.0f, capThrottle);
    int rr = (int)constrain((250.0f * curCommands.RearRightThr), 0.0f, capThrottle);
    analogWrite(k_PinMotorFL, fl);
    analogWrite(k_PinMotorFR, fr);
    analogWrite(k_PinMotorRL, rl);
    analogWrite(k_PinMotorRR, rr);

    // Debug:
#if 0
    Serial.print(curCommands.FrontLeftThr);Serial.print(",\t");
    Serial.print(curCommands.FrontRightThr);Serial.print(",\t");
    Serial.print(curCommands.RearLeftThr);Serial.print(",\t");
    Serial.println(curCommands.RearRightThr);
#endif

#if 0
    Serial.print(fl);Serial.print(",\t");
    Serial.print(fr);Serial.print(",\t");
    Serial.print(rl);Serial.print(",\t");
    Serial.println(rr);
#endif
  }
  // End of the iteration, compute delta, acum total:
  g_DeltaTime = ((float)startTime - (float)millis()) / 1000.0f;
  g_TotalTime += g_DeltaTime;
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
  g_PackedCharacteristic.setValue(0);
  g_StopCharacteristic.setValue(0);

  // Advertise commands service and characteristics:
  BLE.setLocalName("QuadExplorer");
  BLE.setAdvertisedService(g_CommandsService);
  g_CommandsService.addCharacteristic(g_StopCharacteristic);
  g_CommandsService.addCharacteristic(g_PackedCharacteristic);
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
  //IMU.setContinuousMode(); // This enables the FIFO
  
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

void Halt()
{
  FC.Halt();
  
  analogWrite(k_PinMotorFL, 0);
  analogWrite(k_PinMotorFR, 0);
  analogWrite(k_PinMotorRL, 0);
  analogWrite(k_PinMotorRR, 0);

  while(1)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250); 
    digitalWrite(LED_BUILTIN, LOW);
    delay(250); 
  }
}

bool GetRawAccel(float& x, float& y, float& z)
{
  if(!IMU.accelerationAvailable() || !IMU.readAcceleration(x, y, z))
  {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
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
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    return false;
  }
  x += k_GyroXOff;
  y += k_GyroYOff;
  z += k_GyroZOff;
  return true;
}

void GetOrientation(float& yaw, float& pitch, float& roll)
{
  static float k_LastAx = 0.0f;
  static float k_LastAy = 0.0f;
  static float k_LastAz = 1.0f;

  float ax, ay, az;
  if(GetRawAccel(ax, ay, az))
  {   
    k_LastAx = ax;
    k_LastAy = ay;
    k_LastAz = az;
  }
  else
  {
    ax = k_LastAx;
    ay = k_LastAy;
    az = k_LastAz;
    //Serial.println("ERROR READING ACCEL"); // Atm, we should stop the drone if we get this
  }
  float accMagnitude = sqrt((ax*ax) + (ay*ay) + (az*az));
  float rawPitch = atan2((ax / accMagnitude) , (az / accMagnitude)) * RAD_TO_DEG;
  float rawRoll = atan2((-ay / accMagnitude) , (az / accMagnitude)) * RAD_TO_DEG;
  
  // TO-DO: if we detec huge dps, Halt FC.
  static float k_LastWx = 0.0f;
  static float k_LastWy = 0.0f;
  static float k_LastWz = 0.0f;

  float wx, wy, wz;
  if(GetRawGyro(wx, wy, wz))
  {
    k_LastWx = wx;
    k_LastWy = wy;
    k_LastWz = wz;
  }
  else
  {
    wx = k_LastWx;
    wy = k_LastWy;
    wz = k_LastWz;
    //Serial.println("ERROR READING GYR"); // Atm, we should stop the drone if we get this
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
    s_AcumYaw -= wz * g_DeltaTime;
    s_AcumPitch -= wy * g_DeltaTime;
    s_AcumRoll -= wx * g_DeltaTime;
  }
  
  // Transfer angle as we have yawed:
  s_AcumPitch -= s_AcumRoll * sin((-wz * g_DeltaTime) * DEG_TO_RAD);
  s_AcumRoll += s_AcumPitch * sin((-wz * g_DeltaTime) * DEG_TO_RAD);

  // Combine raw accel and gyro, this adds noise but removes gyro drift over time:
  s_AcumPitch = s_AcumPitch * 0.999f + rawPitch * 0.001f;
  s_AcumRoll = s_AcumRoll * 0.999f + rawRoll * 0.001f;

  yaw = s_AcumYaw;
  pitch = -s_AcumPitch;
  roll = -s_AcumRoll;

  // Debug:
#if 0
  Serial.print(pitch); Serial.print(",\t");
  Serial.print(yaw);Serial.print(",\t");
  Serial.println(roll);
#endif
}

void GetControlCommandsRaw(int32_t* throttle, int32_t* yaw, int32_t* pitch, int32_t* roll)
{    
  /*
    result |= ((byte)throttle)     << 0 ;   8 bits (unsigned)
    result |= ((byte)yaw >> 1)     << 8 ;   7 bits (signed)
    result |= ((byte)pitch >> 1)   << 16;   7 bits (signed)
    result |= ((byte)roll >> 1)    << 24    7 bits (signed)

    result |= (yawNegative    ? 1 : 0) << 15;
    result |= (pitchNegative  ? 1 : 0) << 23;
    result |= (rollNegative   ? 1 : 0) << 31;
  */

  uint32_t packed = 0;
  g_PackedCharacteristic.readValue(packed);

  *throttle = (int32_t)((packed >> 0 ) & 0xFF);
  *yaw      = (int32_t)((packed >> 8)  & 0x7F);
  *pitch    = (int32_t)((packed >> 16) & 0x7F);
  *roll     = (int32_t)((packed >> 24) & 0x7F);

  *yaw    *= ((packed >> 15) & 0x1) ? -1 : 1;
  *pitch  *= ((packed >> 23) & 0x1) ? -1 : 1;
  *roll   *= ((packed >> 31) & 0x1) ? -1 : 1;

  // Debug:
#if 0
  Serial.print(*throttle); Serial.print(", \t");
  Serial.print(*yaw); Serial.print(",\t");
  Serial.print(*pitch); Serial.print(",\t");
  Serial.println(*roll);
#endif
}

void GetControlCommands(float& throttle, float& yaw, float& pitch, float& roll)
{
  int32_t rawThrottle, rawYaw, rawPitch, rawRoll;
  GetControlCommandsRaw(&rawThrottle, &rawYaw, &rawPitch, &rawRoll);

  float maxCommand = 10.0f; // Degrees

  throttle = constrain((float)rawThrottle/255.0f, 0.0f, 0.9f); // Clamp so we dont saturate PIDs

  // Reversed to match sim frame or reference:
  yaw = -constrain(((float)rawYaw / 127.0f) * maxCommand, -maxCommand, maxCommand) * DEG_TO_RAD;
  pitch = constrain(((float)rawPitch / 127.0f) * maxCommand, -maxCommand, maxCommand) * DEG_TO_RAD;
  roll = -constrain(((float)rawRoll / 127.0f) * maxCommand, -maxCommand, maxCommand) * DEG_TO_RAD;
}