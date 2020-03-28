#pragma once

struct FCCommands
{
	// Throttle for each motor (0-100)
	float FrontLeftThr;
	float FrontRightThr;
	float RearLeftThr;
	float RearRightThr;
};

struct FCQuadState
{
	float Height;
	float Pitch;
	float Yaw;
	float Roll;
	float DeltaTime;
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
};

class QuadFlyController
{
public:
	QuadFlyController();
	void RenderUI();
	void Reset();
	FCCommands Iterate(const FCQuadState& state);

	float HeightSetPoint;
	float PitchSetPoint;
	float RollSetPoint;
	PID HeightPID = PID(1.3f,0.0f,0.4f);
	PID PitchPID = PID(0.6f, 0.0f, 0.05f);
	PID RollPID = PID(0.6f, 0.0f, 0.05f);
};