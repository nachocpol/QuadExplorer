#include "QuadFlyController.h"

#ifdef _WIN32 
	#include <cmath>
	#include "Graphics/UI/IMGUI/imgui.h"
	#include "glm/glm.hpp"
	#define DEG_TO_RAD  0.017453292519943295769236907684886f
	#define RAD_TO_DEG  57.295779513082320876798154814105f
	#define constrain(x,a,b) glm::clamp(x,a,b);
#else
	#include <Arduino.h>
#endif

QuadFlyController::QuadFlyController()
{
	Reset();
}

void QuadFlyController::RenderUI()
{
#ifdef _WIN32 
	if (ImGui::TreeNode("Height PID"))
	{
		ImGui::InputFloat("Set Point", &HeightSetPoint);
		HeightPID.RenderUI();
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Pitch PID"))
	{
		ImGui::InputFloat("Set Point", &PitchSetPoint);
		PitchPID.RenderUI();
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Roll PID"))
	{
		ImGui::InputFloat("Set Point", &RollSetPoint);
		RollPID.RenderUI();
		ImGui::TreePop();
	}
#endif
}

void QuadFlyController::Reset()
{
	HeightSetPoint = 0.0f;
	RollSetPoint = 0.0f;
	mState = State::Idle;

	HeightPID.Reset();
	PitchPID.Reset();
	RollPID.Reset();
}

FCCommands QuadFlyController::Iterate(const FCQuadState& state, const FCSetPoints& setPoints)
{
	// Check fail safe:
	if(mState != State::FailSafe)
	{
		float pitchDeg = abs(state.Pitch * RAD_TO_DEG);
		float rollDeg = abs(state.Roll * RAD_TO_DEG);
		if (pitchDeg >  45.0f || rollDeg > 45.0f)
		{
			Halt();
		}
	}

	FCCommands commands = {};
	memset(&commands, 0, sizeof(FCCommands));

	bool runPID = false;
	switch (mState)
	{
		case State::Idle:
		{
			runPID = true;
			break;
		}
		case State::FailSafe:
		default:
		{
			runPID = false;
			break;
		}
	}

	// Get PID adjustments:
	if (runPID)
	{
		// Pitch PID
		float pitchAction = 0.0f;
		if (runPID)
		{
			float pitchError = setPoints.Pitch - state.Pitch;
			pitchAction = PitchPID.Get(pitchError, state.DeltaTime);
		}

		// Roll PID
		float rollAction = 0.0f;
		if (runPID)
		{
			float rollError = setPoints.Roll - state.Roll;
			rollAction = RollPID.Get(rollError, state.DeltaTime);
		}

		pitchAction = constrain(pitchAction, -10.0f, 10.0f);
		rollAction = constrain(rollAction, -10.0f, 10.0f);

		commands.FrontLeftThr = setPoints.Thrust - rollAction - pitchAction;
		commands.RearLeftThr = setPoints.Thrust - rollAction + pitchAction;
		
		commands.FrontRightThr = setPoints.Thrust + rollAction - pitchAction;
		commands.RearRightThr = setPoints.Thrust + rollAction + pitchAction;
	}

	return commands;
}

void QuadFlyController::Halt()
{
	mState = State::FailSafe;
}

PID::PID(float kp, float ki, float kd)
	:KP(kp)
	,KI(ki)
	,KD(kd)
{
	Reset();
}

float PID::Get(float error, float deltaTime)
{
	float P = error;
	
	float I = 0.0f;
	
	float D = mFirst ? 0.0f : (error - mPrevError) / deltaTime;
	mFirst = false;
	mPrevError = error;

	LastP = P * KP;
	LastI = I * KI;
	LastD = D * KD;

	return LastP + LastI + LastD;
}

void PID::Reset()
{
	mFirst = true;
	mPrevError = 0.0f;

	LastP = 0.0f;
	LastI = 0.0f;
	LastD = 0.0f;
}

void PID::RenderUI()
{
#ifdef _WIN32 
	ImGui::SliderFloat("KP", &KP, 0.0f, 5.0f);
	ImGui::SliderFloat("KI", &KI, 0.0f, 5.0f);
	ImGui::SliderFloat("KD", &KD, 0.0f, 1.0f);
#endif
}

