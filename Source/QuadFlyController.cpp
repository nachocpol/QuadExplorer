#include "QuadFlyController.h"
#include "Graphics/UI/IMGUI/imgui.h"

QuadFlyController::QuadFlyController()
	:HeightSetPoint(1.5f)
	,RollSetPoint(0.0f)
{
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
	HeightPID.Reset();
	PitchPID.Reset();
	RollPID.Reset();
}

FCCommands QuadFlyController::Iterate(const FCQuadState& state)
{
	FCCommands commands = {};

	// Height PID
	float heightAction = 0.0f;
	{
		float heightError = HeightSetPoint - state.Height;
		heightAction = HeightPID.Get(heightError, state.DeltaTime);
	}

	// Pitch PID
	float pitchAction = 0.0f;
	{
		float pitchError = PitchSetPoint - state.Pitch;
		pitchAction = PitchPID.Get(pitchError, state.DeltaTime);
	}

	// Roll PID
	float rollAction = 0.0f;
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

