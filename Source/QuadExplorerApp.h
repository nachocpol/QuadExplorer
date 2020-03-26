#pragma once

#include "Core/App/AppBase.h"
#include "Graphics/World/SceneGraph.h"
#include "Graphics/TestRenderer.h"

#include "Simulation.h"
#include "Quad.h"
#include "QuadFlyController.h"

namespace World
{
	class Actor;
}
namespace Graphics
{
	struct Model;
}

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
	QuadFlyController mFlyController;
	float mCurTime;
	bool mLoopVisualization;
	int mOverrideSimFrameIndex;

	// Visualization:
	World::Actor* mQuadActor;
	World::Actor* mSimCamera;
	Graphics::Model* mCubeModel;
	Graphics::Model* mQuadModel;
};