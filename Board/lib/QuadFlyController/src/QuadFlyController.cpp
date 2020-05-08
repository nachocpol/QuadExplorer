#include "QuadFlyController.h"

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

		pitchAction = constrain(pitchAction, -1.0f, 1.0f);
		rollAction = constrain(rollAction, -1.0f, 1.0f);

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

#ifdef _WIN32 
void QuadFlyController::QuerySimState(SimulationFrame* simFrame)
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
