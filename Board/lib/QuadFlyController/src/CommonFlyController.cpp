#include "CommonFlyController.h"

#ifdef _WIN32 
	#include "Graphics/UI/IMGUI/imgui.h"
#endif

PID::PID(float kp, float ki, float kd)
	:KP(kp)
	, KI(ki)
	, KD(kd)
{
	Reset();
}

float PID::Get(float error, float deltaTime)
{
	float P = error;

	mIntegral = mIntegral + error * deltaTime;

	float D = mFirst ? 0.0f : (error - mPrevError) / deltaTime;
	mFirst = false;
	mPrevError = error;

	LastP = P * KP;
	LastI = mIntegral * KI;
	LastD = D * KD;

	return LastP + LastI + LastD;
}

void PID::Reset()
{
	mFirst = true;
	mPrevError = 0.0f;
	mIntegral = 0.0f;

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
