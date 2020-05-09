#pragma once

#include "CommonFlyController.h"

class UnityFlyController : public BaseFlyController
{
public:
	UnityFlyController();
	void RenderUI() override;
	void Reset() override;
	FCCommands Iterate(const FCQuadState& state, const FCSetPoints& setPoints) override;
	void Halt() override; 

#ifdef _WIN32 
	void QuerySimState(SimulationFrame* simFrame) override;
#endif

	float HeightSetPoint;
	float PitchSetPoint;
	float RollSetPoint;
	PID HeightPID = PID(0.5f, 0.0f, 0.2f);
	PID PitchPID = PID(0.6f, 0.0f, 0.032f);
	PID RollPID = PID(0.6f, 0.0f, 0.032f);

private:
	struct State
	{
		enum T
		{
			Initial,
			Ascend,
			Traverse
		};
	};
	State::T mState;   // State of the flight controller
};