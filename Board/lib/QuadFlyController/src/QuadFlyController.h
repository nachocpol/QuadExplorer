#pragma once

// Output commands from the flight controller. Motor thrust 0-100.
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
};

class QuadFlyController
{
public:
	QuadFlyController();
	void RenderUI();
	void Reset();
	FCCommands Iterate(const FCQuadState& state, const FCSetPoints& setPoints);
	void Halt();

	float HeightSetPoint;
	float PitchSetPoint;
	float RollSetPoint;
	PID HeightPID = PID(1.3f,0.0f,0.4f);
	PID PitchPID = PID(0.15f, 0.0f, 0.01f);
	PID RollPID = PID(0.15f, 0.0f, 0.01f);

private:
	struct State
	{
		enum T
		{
			Idle,
			Flight,
			FailSafe
		};
	};
	State::T mState;   // State of the flight controller
};