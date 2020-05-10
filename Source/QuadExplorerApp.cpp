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
#include "Graphics/DebugDraw.h"
#include "Core/Logging.h"

QuadExplorerApp::QuadExplorerApp()
	:mCurTime(0.0f)
	,mLoopVisualization(true)
	,mInterpolateFrames(true)
	,mOverrideSimFrameIndex(0)
	,mVisualizationSpeed(1.0f)
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
	mFlyController = new UnityFlyController;
	mSimulation.SetFlightController(mFlyController);

	// Setup visualization:
	mCubeModel = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/cube.obj", mGraphicsInterface);
	glm::mat3 rot = glm::rotate(glm::mat4(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	mQuadModel = Graphics::ModelFactory::Get()->LoadFromFile("assets:Meshes/QuadDrone.FBX", mGraphicsInterface, rot);

	mQuadActor = mScene.SpawnActor();
	mQuadActor->AddComponent<World::TransformComponent>()->SetScale(mQuad.Width,mQuad.Height,mQuad.Depth);
	mQuadActor->AddComponent<World::ModelComponent>()->SetModel(mCubeModel);

	mGroundActor = mScene.SpawnActor();
	auto groundTransform = mGroundActor->AddComponent<World::TransformComponent>();
	groundTransform->SetScale(15.0f, 0.01f, 15.0f);
	groundTransform->SetPosition(0.0, -0.35f, 0.0f);
	mGroundActor->AddComponent<World::ModelComponent>()->SetModel(mCubeModel);

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

	// Future BT:
	// https://docs.microsoft.com/es-es/windows/win32/bluetooth/bluetooth-start-page


	// Query coms:
	{
		char data[128];
		int readBytes = mSerialCom.ReadBytes(data, 128);
		if (readBytes > 0)
		{
			data[readBytes] = 0;
			INFO(data);
		}
	}

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
			mCurTime += DeltaTime * mVisualizationSpeed;
			if (mCurTime > mSimulation.TotalSimTime)
			{
				mCurTime = 0.0f;
			}
			// Get simulation frame:
			simFrame = mSimulation.GetSimulationFrame(mCurTime, mInterpolateFrames);
		}
		else
		{
			simFrame = mSimulation.GetSimulationFrameFromIdx(mOverrideSimFrameIndex);
		}

		mQuadActor->Transform->SetPosition(simFrame.QuadPosition);
		mQuadActor->Transform->SetRotation(simFrame.QuadOrientation);
		Graphics::DebugDraw::GetInstance()->DrawLine(simFrame.QuadPosition, simFrame.QuadPosition + glm::normalize(simFrame.WorldForce));
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

	mSerialCom.RenderUI();

	// Render all the UI to show and tweak values:
	ImGui::Begin("Quad Explorer");
	{
		if (ImGui::Button("Run Simulation"))
		{
			mSimulation.RunSimulation();
			mCurTime = 0.0f;
			mOverrideSimFrameIndex = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			mCurTime = 0.0f;
			mOverrideSimFrameIndex = 0;
		}
		ImGui::Checkbox("Loop Visualization", &mLoopVisualization);
		if (mLoopVisualization)
		{
			ImGui::Text("%f/%f", mCurTime, mSimulation.TotalSimTime);
			ImGui::SliderFloat("Playback Speed", &mVisualizationSpeed, 0.0f, 2.0f);
		}
		else
		{
			ImGui::Text("%f/%f", mOverrideSimFrameIndex * mSimulation.DeltaTime, mSimulation.TotalSimTime);
			ImGui::SliderInt("Sim Frame", &mOverrideSimFrameIndex, 0, mSimulation.GetNumFrames() - 1);
		}
		ImGui::Checkbox("Interpolate Frames", &mInterpolateFrames);
		ImGui::Separator();

		bool simReady = mSimulation.HasResults();
		if (simReady)
		{
			ImGui::Begin("Plotting");
			if (ImGui::CollapsingHeader("General"))
			{
				// Plot height
				ImGui::PlotLines("Height", [](void* data, int idx) 
				{
					const SimulationResult* results = (const SimulationResult*)data;
					if (!results)
					{
						return 0.0f;
					}
					return results->Frames[idx].QuadPosition.y;
				}, (void*)&mSimulation.GetSimulationResults(), mSimulation.GetNumFrames(), 0, 0, -1.0f, 5.0f, ImVec2(512, 128));

				// Plot pitch
				ImGui::PlotLines("Pitch", [](void* data, int idx)
				{
					const SimulationResult* results = (const SimulationResult*)data;
					if (!results)
					{
						return 0.0f;
					}
					return glm::degrees(results->Frames[idx].QuadOrientation.x);
				}, (void*)&mSimulation.GetSimulationResults(), mSimulation.GetNumFrames(), 0, 0,-60, 60.0f, ImVec2(512, 128));

				// Plot roll
				ImGui::PlotLines("Roll", [](void* data, int idx)
				{
					const SimulationResult* results = (const SimulationResult*)data;
					if (!results)
					{
						return 0.0f;
					}
					return glm::degrees(results->Frames[idx].QuadOrientation.z);
				}, (void*)&mSimulation.GetSimulationResults(), mSimulation.GetNumFrames(), 0, 0, -60.0f, 60.0f, ImVec2(512, 128));
			}
			if (ImGui::CollapsingHeader("PID"))
			{
				struct PIDDATA
				{
					const SimulationResult* Results;
					SimulationFrame::PIDType Type;
				};

				auto plotPID_P = [this](const char* label, PIDDATA data) {
					ImGui::PlotLines(label, [](void* data, int idx)
					{
						const PIDDATA* pData = (const PIDDATA*)data;
						if (!pData)
						{
							return 0.0f;
						}
						return pData->Results->Frames[idx].GetPIDState(pData->Type).P;
					}, (void*)&data, mSimulation.GetNumFrames(), 0, 0, FLT_MAX, FLT_MAX, ImVec2(512, 128));
				};
				auto plotPID_I = [this](const char* label, PIDDATA data) {
					ImGui::PlotLines(label, [](void* data, int idx)
					{
						const PIDDATA* pData = (const PIDDATA*)data;
						if (!pData)
						{
							return 0.0f;
						}
						return pData->Results->Frames[idx].GetPIDState(pData->Type).I;
					}, (void*)&data, mSimulation.GetNumFrames(), 0, 0, FLT_MAX, FLT_MAX, ImVec2(512, 128));
				};
				auto plotPID_D = [this](const char* label, PIDDATA data) {
					ImGui::PlotLines(label, [](void* data, int idx)
					{
						const PIDDATA* pData = (const PIDDATA*)data;
						if (!pData)
						{
							return 0.0f;
						}
						return pData->Results->Frames[idx].GetPIDState(pData->Type).D;
					}, (void*)&data, mSimulation.GetNumFrames(), 0, 0, FLT_MAX, FLT_MAX, ImVec2(512, 128));
				};

				PIDDATA dat = { &mSimulation.GetSimulationResults(), SimulationFrame::PIDType::Height };
				if (ImGui::CollapsingHeader("Height"))
				{
					plotPID_P("P", dat);
					plotPID_I("I", dat);
					plotPID_D("D", dat);
				}
				dat.Type = SimulationFrame::Pitch;
				if (ImGui::CollapsingHeader("Pitch"))
				{
					plotPID_P("P", dat);
					plotPID_I("I", dat);
					plotPID_D("D", dat);
				}
				dat.Type = SimulationFrame::Roll;
				if (ImGui::CollapsingHeader("Roll"))
				{
					plotPID_P("P", dat);
					plotPID_I("I", dat);
					plotPID_D("D", dat);
				}
			}

			ImGui::End();
		}

		// Display simulation UI (this will also show quad UI)
		if (ImGui::CollapsingHeader("Simulation"))
		{
			mSimulation.RenderUI();
		}
		if (ImGui::CollapsingHeader("Fly Controller"))
		{
			mFlyController->RenderUI();
		}
	}
	ImGui::End();
}

QuadExplorerApp app;
ENTRY_POINT(app, "Quad Explorer App", false);
