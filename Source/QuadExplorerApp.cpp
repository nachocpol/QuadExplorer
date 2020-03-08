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
}

void QuadExplorerApp::Update()
{
	AppBase::Update();

	ImGui::Begin("Quad Explorer");
	ImGui::End();

	mScene.Update(DeltaTime);
	mRenderer.Render(&mScene);
}

void QuadExplorerApp::Release()
{
	AppBase::Release();
}

QuadExplorerApp app;
ENTRY_POINT(app, "Quad Explorer App", false);
