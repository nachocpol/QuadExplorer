#include "Simulation.h"
#include "Quad.h"
#include "QuadFlyController.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/World/PhysicsWorld.h"
#include "Core/Logging.h"

#include "PxPhysicsAPI.h"
#include "PxFiltering.h"
#include "extensions/PxRigidBodyExt.h"

using namespace physx;


Simulation::Simulation()
	:TotalSimTime(5.0f)
	,DeltaTime(0.05f)
	,mQuadTarget(nullptr)
	,mFlightController(nullptr)
{
}

void Simulation::Init()
{
}

void Simulation::SetQuadTarget(Quad* quad)
{
	mQuadTarget = quad;
}

void Simulation::SetFlightController(QuadFlyController* fc)
{
	mFlightController = fc;
}

void Simulation::RenderUI()
{
	ImGui::InputFloat("Total Simulation Time", &TotalSimTime);
	ImGui::InputFloat("Delta Time", &DeltaTime);
	int numberSteps = TotalSimTime / DeltaTime;
	ImGui::Text("Total Simulation Steps: %i", numberSteps);

	if (mQuadTarget)
	{
		if (ImGui::TreeNode("Quad"))
		{
			mQuadTarget->RenderUI();
			ImGui::TreePop();
		}
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
	rigidBody->setMass(mQuadTarget->Mass);

	physxScene->addActor(*rigidBody);

	// Run each simulation step:
	for (int frameIdx = 0; frameIdx < numSimFrames; ++frameIdx)
	{
		SimulationFrame& frame = mResult.Frames[frameIdx];

		// Advance sim:
		mQuadTarget->Position.y = rigidBody->getGlobalPose().p.y;

		// FC, run current iteration:
		FCQuadState fcState;
		fcState.DeltaTime = DeltaTime;
		fcState.Height = mQuadTarget->Position.y;
		FCCommands fcCommands = mFlightController->Iterate(fcState);

		// Apply quad commands
		float tmpForce = fcCommands.FrontLeftThr * 4.0f; // 4.0Newtowns max force?
		rigidBody->addForce(PxVec3(0.0f, tmpForce, 0.0f),PxForceMode::eFORCE);

		// Query sim state, used for the 3D visualization:
		frame.QuadOrientation = mQuadTarget->Orientation;
		frame.QuadPosition = mQuadTarget->Position;

		// Step the physx simulation:
		physxScene->simulate(DeltaTime);
		physxScene->fetchResults(true);

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
