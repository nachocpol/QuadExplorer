#include "UnityFlightController.h"

#ifdef _WIN32 
	#include "Simulation.h"
	#include <cmath>
	#include "Graphics/UI/IMGUI/imgui.h"
	#include "glm/glm.hpp"
	#define DEG_TO_RAD  0.017453292519943295769236907684886f
	#define RAD_TO_DEG  57.295779513082320876798154814105f
	#define constrain(x,a,b) glm::clamp(x,a,b);
#else
	#include <Arduino.h>
#endif

UnityFlyController::UnityFlyController()
{
	Reset();
}

void UnityFlyController::RenderUI()
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

void UnityFlyController::Reset()
{
	HeightSetPoint = 0.0f;
	RollSetPoint = 0.0f;
	mState = State::Initial;

	HeightPID.Reset();
	PitchPID.Reset();
	RollPID.Reset();
}

FCCommands UnityFlyController::Iterate(const FCQuadState& state, const FCSetPoints& setPoints)
{
	FCCommands commands = {};
	memset(&commands, 0, sizeof(FCCommands));

	float HeightSetPoint = 0.0f;
	float RollSetPoint = 0.0f;
	float PitchSetPoint = 0.0f;

	bool runPID = false;
	switch (mState)
	{
	case State::Initial:
	{
		if (state.Time > 1.5f)
		{
			mState = State::Ascend;
		}
		break;
	}
	case State::Ascend:
	{
		PitchSetPoint = 20.0f * DEG_TO_RAD;
		if (state.Time > 5.0f)
		{
			RollSetPoint = -20.0 * DEG_TO_RAD;
		}
		runPID = true;
		HeightSetPoint = 3.0f;
		break;
	}
	default:
	{
		runPID = false;
		break;
	}
	}

	// Get PID adjustments:
	if (runPID)
	{
		// Height action
		float heightAction = 0.0f;
		if (runPID)
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

		pitchAction = constrain(pitchAction, -1.0f, 1.0f);
		rollAction = constrain(rollAction, -1.0f, 1.0f);
		heightAction = constrain(heightAction, 0.0f, 0.9f);

		commands.FrontLeftThr = heightAction - rollAction - pitchAction;
		commands.RearLeftThr = heightAction - rollAction + pitchAction;

		commands.FrontRightThr = heightAction + rollAction - pitchAction;
		commands.RearRightThr = heightAction + rollAction + pitchAction;
	}

	return commands;
}

void UnityFlyController::Halt()
{
	//mState = State::FailSafe;
}

#ifdef _WIN32 
void UnityFlyController::QuerySimState(SimulationFrame* simFrame)
{
	simFrame->HeightPIDState.P = HeightPID.LastP;
	simFrame->HeightPIDState.I = HeightPID.LastI;
	simFrame->HeightPIDState.D = HeightPID.LastD;

	simFrame->PitchPIDState.P = PitchPID.LastP;
	simFrame->PitchPIDState.I = PitchPID.LastI;
	simFrame->PitchPIDState.D = PitchPID.LastD;

	simFrame->RollPIDState.P = RollPID.LastP;
	simFrame->RollPIDState.I = RollPID.LastI;
	simFrame->RollPIDState.D = RollPID.LastD;
}
#endif
