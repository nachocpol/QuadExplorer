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
	float DeltaTime;
};

class PID
{
public:
	float Get(float error, float deltaTime);
	void Reset();
	void RenderUI();
	float KP = 1.9f;
	float KI = 0.0f;
	float KD = 0.1f;

private:
	bool mFirst = true;
	float mPrevError = 0.0f;
};

class QuadFlyController
{
public:
	QuadFlyController();
	void RenderUI();
	FCCommands Iterate(const FCQuadState& state);

	float HeightSetPoint;
	PID HeightPID;
};