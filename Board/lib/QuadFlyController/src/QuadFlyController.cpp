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
}

void QuadFlyController::Reset()
{
	mState = State::Idle;

	PitchPID.Reset();
	RollPID.Reset();
	YawPID.Reset();
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

		// Yaw PID
		float yawAction = 0.0f;
		if (runPID)
		{
			float yawError = setPoints.Yaw - state.Yaw;
			yawAction = YawPID.Get(yawError, state.DeltaTime);
		}

		pitchAction = constrain(pitchAction, -1.0f, 1.0f);
		rollAction = constrain(rollAction, -1.0f, 1.0f);
		yawAction = constrain(yawAction, -1.0f, 1.0f);

		commands.FrontLeftThr = setPoints.Thrust - rollAction - pitchAction + yawAction;
		commands.RearLeftThr = setPoints.Thrust - rollAction + pitchAction - yawAction;
		
		commands.FrontRightThr = setPoints.Thrust + rollAction - pitchAction - yawAction;
		commands.RearRightThr = setPoints.Thrust + rollAction + pitchAction + yawAction;
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


	simFrame->PitchPIDState.P = PitchPID.LastP;
	simFrame->PitchPIDState.I = PitchPID.LastI;
	simFrame->PitchPIDState.D = PitchPID.LastD;

	simFrame->RollPIDState.P = RollPID.LastP;
	simFrame->RollPIDState.I = RollPID.LastI;
	simFrame->RollPIDState.D = RollPID.LastD;
}
#endif
