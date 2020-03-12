#include "Simulation.h"
#include "Quad.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/World/PhysicsWorld.h"
#include "Core/Logging.h"

#include "PxPhysicsAPI.h"
#include "PxFiltering.h"
#include "extensions/PxRigidBodyExt.h"

using namespace physx;


Simulation::Simulation():
	 TotalSimTime(5.0f)
	,DeltaTime(0.05f)
	,mQuadTarget(nullptr)
{
}

void Simulation::Init()
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

	// Setup the physx scene:
	auto physx = World::PhysicsWorld::GetInstance()->GetPhyx();
	PxSceneDesc sceneDesc = PxSceneDesc(physx->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = World::PhysicsWorld::GetInstance()->GetPhyxCPUDispatcher();
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	physx::PxScene* physxScene = physx->createScene(sceneDesc);

	// Quad rigidbody stuff:
	PxTransform quadInitialTransform;
	quadInitialTransform.p = PxVec3(0.0f, 0.0f, 0.0f);
	quadInitialTransform.q = PxQuat(0.0f, 0.0f, 0.0f, 1.0f);
	auto rigidBody = physx->createRigidDynamic(quadInitialTransform);
	rigidBody->setMass(0.25f);

	physxScene->addActor(*rigidBody);

	// Run each simulation step:
	for (int frameIdx = 0; frameIdx < numSimFrames; ++frameIdx)
	{
		SimulationFrame& frame = mResult.Frames[frameIdx];

		// Advance sim:
		mQuadTarget->Position.y = rigidBody->getGlobalPose().p.y;

		// Query sim state:
		frame.QuadOrientation = mQuadTarget->Orientation;
		frame.QuadPosition = mQuadTarget->Position;

		curTime += DeltaTime;

		// Step the physx simulation:
		physxScene->simulate(DeltaTime);
		physxScene->fetchResults(true);
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
