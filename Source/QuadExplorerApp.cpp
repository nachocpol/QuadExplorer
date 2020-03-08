#include "QuadExplorerApp.h"

#include "Core/EntryPoint.h"
#include "Core/FileSystem.h"

#include "Graphics/UI/IMGUI/imgui.h"

QuadExplorerApp::QuadExplorerApp()
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

}

void QuadExplorerApp::Update()
{
	AppBase::Update();

	RenderUI();

	mScene.Update(DeltaTime);
	mRenderer.Render(&mScene);
}

void QuadExplorerApp::Release()
{
	AppBase::Release();
}

void QuadExplorerApp::RenderUI()
{
	// Main U
	ImGui::Begin("Quad Explorer");

	if (ImGui::Button("Run Simulation"))
	{

	}
	ImGui::End();

	// Display sim UI (this will also show quad UI)
	mSimulation.RenderUI();
}

QuadExplorerApp app;
ENTRY_POINT(app, "Quad Explorer App", false);
