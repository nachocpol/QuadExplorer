#include "QuadFlyController.h"
#include "Graphics/UI/IMGUI/imgui.h"

QuadFlyController::QuadFlyController():
	HeightSetPoint(1.5f)
{
}

void QuadFlyController::RenderUI()
{
	ImGui::Begin("FC");
	ImGui::InputFloat("Height Set Point", &HeightSetPoint);
	ImGui::End();
}

FCCommands QuadFlyController::Iterate(const FCQuadState& state)
{
	FCCommands commands = {};

	// Height PID
	{
		float hError = HeightSetPoint - state.Height;
		float action = HeightPID.Get(hError, state.DeltaTime);

		commands.FrontLeftThr  = action;
		commands.FrontRightThr = action;
		commands.RearLeftThr = action;
		commands.RearRightThr = action;
	}

	return commands;
}

float PID::Get(float error, float deltaTime)
{
	float P = error;
	float I = 0.0f;
	float D = 0.0f;
	return P * KP + I * KI + D * KD;
}
