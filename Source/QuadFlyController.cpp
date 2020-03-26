#include "QuadFlyController.h"
#include "Graphics/UI/IMGUI/imgui.h"

QuadFlyController::QuadFlyController():
	HeightSetPoint(1.5f)
{
}

void QuadFlyController::RenderUI()
{
	ImGui::InputFloat("Height Set Point", &HeightSetPoint);
	if (ImGui::TreeNode("Height PID"))
	{
		HeightPID.RenderUI();
		ImGui::TreePop();
	}
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
	
	float D = mFirst ? 0.0f : (error - mPrevError) / deltaTime;
	mFirst = false;
	mPrevError = error;

	return P * KP + I * KI + D * KD;
}

void PID::Reset()
{
	mFirst = true;
	mPrevError = 0.0f;
}

void PID::RenderUI()
{
	ImGui::InputFloat("KP", &KP);
	ImGui::InputFloat("KI", &KI);
	ImGui::InputFloat("KD", &KD);
}

