#pragma once

#include "Core/App/AppBase.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/TestRenderer.h"

class QuadExplorerApp : public AppBase
{
public:
	QuadExplorerApp();
	~QuadExplorerApp();
	void Init() final override;
	void Update() final override;
	void Release()final override;

private:
	World::SceneGraph mScene;
	Graphics::TestRenderer mRenderer;
};