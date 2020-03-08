#pragma once

#include "Core/App/AppBase.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/TestRenderer.h"

#include "Simulation.h"
#include "Quad.h"

class QuadExplorerApp : public AppBase
{
public:
	QuadExplorerApp();
	~QuadExplorerApp();
	void Init() final override;
	void Update() final override;
	void Release()final override;

	void RenderUI();

private:
	World::SceneGraph mScene;
	Graphics::TestRenderer mRenderer;

	Simulation mSimulation;
	Quad mQuad;
};