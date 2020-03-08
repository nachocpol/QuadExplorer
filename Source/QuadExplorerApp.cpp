#include "QuadExplorerApp.h"

#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"
#include "Graphics/World/CameraComponent.h"
#include "Graphics/World/Actor.h"
#include "Graphics/World/Model.h"
#include "Graphics/World/TransformComponent.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/Platform/BaseWindow.h"

QuadExplorerApp::QuadExplorerApp():
	 mCurTime(0.0f)
	,mLoopVisualization(true)
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
	mSimulation.SetQuadTarget(&mQuad);

	// Setup visualization:
	mCubeModel = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/cube.obj", mGraphicsInterface);
	
	mQuadActor = mScene.SpawnActor();
	mQuadActor->AddComponent<World::TransformComponent>();
	mQuadActor->AddComponent<World::ModelComponent>()->SetModel(mCubeModel);

	float aspect = (float)mWindow->GetWidth() / (float)mWindow->GetHeight();
	mSimCamera = mScene.SpawnActor();
	mSimCamera->AddComponent<World::TransformComponent>();
	mSimCamera->AddComponent<World::CameraComponent>()->ConfigureProjection(aspect, 80.0f, 0.1f, 100.0f);
	mSimCamera->Transform->SetPosition(-5.0f, 1.0f, 0.0f);
}

void QuadExplorerApp::Update()
{
	AppBase::Update();

	RenderUI();

	// Process visualization:
	if (mSimulation.HasResults())
	{
		if (mLoopVisualization)
		{
			// Sim time:
			mCurTime += DeltaTime;
			if (mCurTime > mSimulation.TotalSimTime)
			{
				mCurTime = 0.0f;
			}
		}
		// Get simulation frame:
		SimulationFrame simFrame = mSimulation.GetSimulationFrame(mCurTime);

		mQuadActor->Transform->SetPosition(simFrame.QuadPosition);
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
	ImGui::Begin("Quad Explorer");

	if (ImGui::Button("Run Simulation"))
	{
		mSimulation.RunSimulation();
	}
	ImGui::Checkbox("Loop Visualization", &mLoopVisualization);
	ImGui::End();

	// Display sim UI (this will also show quad UI)
	mSimulation.RenderUI();
}

QuadExplorerApp app;
ENTRY_POINT(app, "Quad Explorer App", false);
