#include "Simulation.h"
#include "Quad.h"
#include "Graphics/UI/IMGUI/imgui.h"

Simulation::Simulation():
	 TotalSimTime(5.0f)
	,DeltaTime(0.05f)
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

void Simulation::RunSimulation()
{
	// Setup simulation:
	int numSimFrames = TotalSimTime / DeltaTime;
	mResult.DeltaTime = DeltaTime;
	mResult.Frames.clear();
	mResult.Frames.resize(numSimFrames);
	float curTime = 0.0f;
	mQuadTarget->Reset();

	// Run each simulation step:
	for (int frameIdx = 0; frameIdx < numSimFrames; ++frameIdx)
	{
		SimulationFrame& frame = mResult.Frames[frameIdx];

		// Advance sim:
		mQuadTarget->Position.y = glm::sin(curTime) * 2.0f;

		// Query sim state:
		frame.QuadOrientation = mQuadTarget->Orientation;
		frame.QuadPosition = mQuadTarget->Position;

		curTime += DeltaTime;
	}
}

SimulationFrame Simulation::GetSimulationFrame(float simTime)
{
	// TO-DO: implement interpolation
	int index = (int)((simTime / TotalSimTime) * (float)mResult.Frames.size());
	return mResult.Frames[index];
}

SimulationFrame Simulation::GetSimulationFrameFromIdx(int index)
{
	return mResult.Frames[index];
}

bool Simulation::HasResults() const
{
	return !mResult.Frames.empty();
}

int Simulation::GetNumFrames()
{
	if (HasResults())
	{
		return mResult.Frames.size();
	}
	return 0;
}
