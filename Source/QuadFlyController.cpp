#include "QuadFlyController.h"
#include <cmath>
#include "Graphics/UI/IMGUI/imgui.h"

QuadFlyController::QuadFlyController()
{
	Reset();
}

void QuadFlyController::RenderUI()
{
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
}

void QuadFlyController::Reset()
{
	mReachedTop = false;
	mCountDown = 2.0f;
	HeightSetPoint = 1.5f;
	RollSetPoint = 0.0f;
	mState = State::Initial;

	HeightPID.Reset();
	PitchPID.Reset();
	RollPID.Reset();
}

FCCommands QuadFlyController::Iterate(const FCQuadState& state)
{
	FCCommands commands = {};

	bool runPID = false;

	// Check fail safe:
	if(mState != State::FailSafe)
	{
		if (abs(state.Pitch) > 35.0f || abs(state.Roll) > 35.0f)
		{
			mState = State::FailSafe;
		}
	}

	switch (mState)
	{
		case State::Initial:
		{
			if (state.Time > 3.0f)
			{
				mState = State::Climbing;
			}
			break;
		}

		case State::Climbing:
		{
			runPID = true;

			if (!mReachedTop)
			{
				float targetThreshold = 0.35f; // Reached target if within 10cm
				if ((HeightSetPoint - state.Height) <= targetThreshold)
				{
					mReachedTop = true;
				}
			}
			else
			{
				mCountDown -= state.DeltaTime;
				if (mCountDown <= 0.0f)
				{
					mState = State::Descending;
				}
			}
			break;
		}

		case State::Descending:
		{
			runPID = true;

			if (HeightSetPoint > 0.0f)
			{
				HeightSetPoint -= 0.20f * state.DeltaTime; // Descend at 20cm per second
			}

			if (state.Height <= 0.1f)
			{
				mState = State::Done;
			}

			break;
		}

		case State::Done:
		{
			runPID = false;

			break;
		}

		case State::FailSafe:
		default:
		{
			runPID = false;

			break;
		}
	}

	// Height PID
	float heightAction = 0.0f;
	if(runPID)
	{
		float heightError = HeightSetPoint - state.Height;
		heightAction = HeightPID.Get(heightError, state.DeltaTime);
	}

	// Pitch PID
	float pitchAction = 0.0f;
	if (runPID)
	{
		float pitchError = PitchSetPoint - state.Pitch;
		pitchAction = PitchPID.Get(pitchError, state.DeltaTime);
	}

	// Roll PID
	float rollAction = 0.0f;
	if (runPID)
	{
		float rollError = RollSetPoint - state.Roll;
		rollAction = RollPID.Get(rollError, state.DeltaTime);
	}

	commands.FrontLeftThr = heightAction - rollAction - pitchAction;
	commands.RearLeftThr = heightAction - rollAction + pitchAction;
	
	commands.FrontRightThr = heightAction + rollAction - pitchAction;
	commands.RearRightThr = heightAction + rollAction + pitchAction;

	return commands;
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
	ImGui::SliderFloat("KP", &KP, 0.0f, 5.0f);
	ImGui::SliderFloat("KI", &KI, 0.0f, 5.0f);
	ImGui::SliderFloat("KD", &KD, 0.0f, 1.0f);
}

