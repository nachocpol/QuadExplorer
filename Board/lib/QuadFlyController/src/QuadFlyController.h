#pragma once

#include "CommonFlyController.h"

class QuadFlyController : public BaseFlyController
{
public:
	QuadFlyController();
	void RenderUI() override;
	void Reset() override;
	FCCommands Iterate(const FCQuadState& state, const FCSetPoints& setPoints) override;
	void Halt() override;

#ifdef _WIN32 
	void QuerySimState(SimulationFrame* simFrame) override;
#endif

	PID PitchPID = PID(0.121f, 0.0f, 0.016f);
	PID RollPID = PID(0.121f, 0.0f, 0.016f);
	PID YawPID = PID(0.121f, 0.0f, 0.0f);

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