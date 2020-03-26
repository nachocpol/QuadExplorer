#include "QuadExplorerApp.h"

#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"
#include "Graphics/World/CameraComponent.h"
#include "Graphics/World/Actor.h"
#include "Graphics/World/Model.h"
#include "Graphics/World/TransformComponent.h"
#include "Graphics/World/ProbeComponent.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/Platform/BaseWindow.h"

QuadExplorerApp::QuadExplorerApp():
	 mCurTime(0.0f)
	,mLoopVisualization(true)
	,mOverrideSimFrameIndex(0)
{
}

QuadExplorerApp::~QuadExplorerApp()
{
}

void QuadExplorerApp::Init()
{
	// File devices:
	Core::FileSystem::GetInstance()->AddFileDevice({ "../../Depen/AwesomeEngine/Assets/",Core::FileDevice::Type::Assets,"assets" });
	Core::FileSystem::GetInstance()->AddFileDevice({ "../../Depen/AwesomeEngine/Assets/Shaders/HLSL/",Core::FileDevice::Type::ShaderSource,"shadersrc" });

	AppBase::Init();

	mRenderer.Initialize(this);
	mScene.Initialize();

	// Setup simulation and quad:
	mSimulation.Init();
	mSimulation.SetQuadTarget(&mQuad);
	mSimulation.SetFlightController(&mFlyController);

	// Setup visualization:
	mCubeModel = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/cube.obj", mGraphicsInterface);
	glm::mat3 rot = glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mQuadModel = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/QuadDrone.FBX", mGraphicsInterface, rot);

	mQuadActor = mScene.SpawnActor();
	mQuadActor->AddComponent<World::TransformComponent>();
	mQuadActor->AddComponent<World::ModelComponent>()->SetModel(mQuadModel);

	// Add a default probe:
	mScene.SpawnActor()->AddComponent<World::ProbeComponent>();

	// Setup a camera to render the visualization:
	float aspect = (float)mWindow->GetWidth() / (float)mWindow->GetHeight();
	mSimCamera = mScene.SpawnActor();
	mSimCamera->AddComponent<World::TransformComponent>();
	mSimCamera->AddComponent<World::CameraComponent>()->ConfigureProjection(aspect, 80.0f, 0.1f, 100.0f);
	mSimCamera->Transform->SetPosition(-5.0f, 1.0f, 0.0f);
}

void QuadExplorerApp::Update()
{
	AppBase::Update();

	// Render main config UI (also Quad and Sim UI):
	RenderUI();

	// Process visualization:
	if (mSimulation.HasResults())
	{
		// Get current simulation frame (either from time or index)
		SimulationFrame simFrame;
		if (mLoopVisualization)
		{
			// Sim time:
			mCurTime += DeltaTime;
			if (mCurTime > mSimulation.TotalSimTime)
			{
				mCurTime = 0.0f;
			}
			// Get simulation frame:
			simFrame = mSimulation.GetSimulationFrame(mCurTime);
		}
		else
		{
			simFrame = mSimulation.GetSimulationFrameFromIdx(mOverrideSimFrameIndex);
		}

		mQuadActor->Transform->SetPosition(simFrame.QuadPosition);

		// Display cur frame info:
		ImGui::Begin("Quad Explorer");
		ImGui::Text("Position x=%.3f y=%.3f z=%.3f", simFrame.QuadPosition.x, simFrame.QuadPosition.y, simFrame.QuadPosition.z);
		ImGui::End();
	}


	// Trigger scene update and renderer:
	mScene.Update(DeltaTime);
	mRenderer.Render(&mScene);
}

void QuadExplorerApp::Release()
{
	AppBase::Release();
}

void QuadExplorerApp::RenderUI()
{
	bool t = 1;
	ImGui::ShowDemoWindow(&t);

	// Render all the UI to show and tweak values:
	ImGui::Begin("Quad Explorer");
	{
		if (ImGui::Button("Run Simulation"))
		{
			mSimulation.RunSimulation();
		}
		ImGui::Checkbox("Loop Visualization", &mLoopVisualization);
		if (mLoopVisualization)
		{
			ImGui::Text("%f/%f", mCurTime, mSimulation.TotalSimTime);
		}
		else
		{
			ImGui::SliderInt("Sim Frame", &mOverrideSimFrameIndex, 0, mSimulation.GetNumFrames() - 1);
		}

		float t[] = { -1, 0, 1, 0, -1 ,0 };
		ImGui::PlotLines("Test", t, 6,0,0,-1,1);

		// Display simulation UI (this will also show quad UI)
		if (ImGui::CollapsingHeader("Simulation"))
		{
			mSimulation.RenderUI();
		}
		if (ImGui::CollapsingHeader("Fly Controller"))
		{
			mFlyController.RenderUI();
		}
	}
	ImGui::End();
}

QuadExplorerApp app;
ENTRY_POINT(app, "Quad Explorer App", false);
