#include "Simulation.h"
#include "Quad.h"
#include "Graphics/UI/IMGUI/imgui.h"

Simulation::Simulation():
	 TotalSimTime(5.0f)
	,DeltaTime(0.1f)
	,mQuadTarget(nullptr)
{
}

void Simulation::SetQuadTarget(Quad* quad)
{
	mQuadTarget = quad;
}

void Simulation::RenderUI()
{
	ImGui::Begin("Simulation");
	ImGui::InputFloat("Total Simulation Time", &TotalSimTime);
	ImGui::InputFloat("Delta Time", &DeltaTime);
	int numberSteps = TotalSimTime / DeltaTime;
	ImGui::Text("Total Simulation Steps: %i", numberSteps);
	ImGui::End();

	if (mQuadTarget)
	{
		mQuadTarget->RenderUI();
	}
}

