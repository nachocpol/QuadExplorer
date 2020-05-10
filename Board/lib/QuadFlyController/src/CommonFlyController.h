#pragma once

// Output commands from the flight controller. Motor thrust 0-1.
struct FCCommands
{
	float FrontLeftThr;
	float FrontRightThr;
	float RearLeftThr;
	float RearRightThr;
};

// Requested set points, the FC will generate commands to reach these set points. Units: radians
struct FCSetPoints
{
	float Thrust;
	float Yaw;
	float Pitch;
	float Roll;
};

// State required to iterate and generate commands. Units: meters, radians, seconds.
struct FCQuadState
{
	float Height;
	float Pitch;
	float Yaw;
	float Roll;
	float DeltaTime;
	float Time;
};

class PID
{
public:
	PID(float kp, float ki, float kd);
	float Get(float error, float deltaTime);
	void Reset();
	void RenderUI();
	float KP;
	float KI;
	float KD;

	float LastP;
	float LastI;
	float LastD;

private:
	bool mFirst = true;
	float mPrevError = 0.0f;
	float mIntegral = 0.0f;
};

struct SimulationFrame;
class BaseFlyController
{
public:
	BaseFlyController() {}
	virtual ~BaseFlyController() {}
	virtual void RenderUI() = 0;
	virtual void Reset() = 0;
	virtual FCCommands Iterate(const FCQuadState& state, const FCSetPoints& setPoints) = 0;
	virtual void Halt() = 0;

#ifdef _WIN32 
	virtual void QuerySimState(SimulationFrame* simFrame) = 0;
#endif
};