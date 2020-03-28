#define NOMINMAX

#include "Simulation.h"
#include "Quad.h"
#include "QuadFlyController.h"
#include "Graphics/UI/IMGUI/imgui.h"
#include "Graphics/World/PhysicsWorld.h"
#include "Core/Logging.h"

#include "PxPhysicsAPI.h"
#include "PxFiltering.h"
#include "extensions/PxRigidBodyExt.h"

#include "glm/ext.hpp"

using namespace physx;


Simulation::Simulation()
	:TotalSimTime(15.0f)
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
	mFlightController->Reset();

	// Setup the physx scene:
	auto physx = World::PhysicsWorld::GetInstance()->GetPhyx();
	PxSceneDesc sceneDesc = PxSceneDesc(physx->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = World::PhysicsWorld::GetInstance()->GetPhyxCPUDispatcher();
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	PxScene* physxScene = physx->createScene(sceneDesc);

	// Quad rigidbody:
	PxTransform quadInitialTransform;
	quadInitialTransform.p = PxVec3(0.0f, 0.0f, 0.0f);
	glm::quat initialQuat = glm::quat(glm::vec3(glm::radians(20.0f),0.0f,0.0f));
	quadInitialTransform.q = PxQuat(initialQuat.x, initialQuat.y, initialQuat.z, initialQuat.w);
	auto rigidBody = physx->createRigidDynamic(quadInitialTransform);
	rigidBody->setMass(mQuadTarget->Mass);

	// Shape
	float dimX = 0.2f;
	float dimY = 0.05f;
	float dimZ = 0.2f;

	PxMaterial* quadMat = physx->createMaterial(0.0f, 0.0f, 1.0f);
	PxShape* quadBox = physx->createShape(PxBoxGeometry(dimX, dimY, dimZ), *quadMat);
	rigidBody->attachShape(*quadBox);

	float density = mQuadTarget->Mass / (dimX * 2.0f * dimZ * 2.0f * dimY * 2.0f);
	auto tensor = rigidBody->getMassSpaceInertiaTensor();
	PxRigidBodyExt::updateMassAndInertia(*rigidBody, density);
	tensor = rigidBody->getMassSpaceInertiaTensor();

	float checkmass = rigidBody->getMass();

	physxScene->addActor(*rigidBody);

	float rnd1 = 0.2f;
	float rnd2 = -0.375f;
	float rnd3 = 0.33f;
	float rnd4 = -0.3f;

	// Run each simulation step:
	for (int frameIdx = 0; frameIdx < numSimFrames; ++frameIdx)
	{
		// Advance sim:
		PxTransform curTransform = rigidBody->getGlobalPose();
		mQuadTarget->Position = glm::vec3(curTransform.p.x, curTransform.p.y, curTransform.p.z);
		mQuadTarget->Orientation = glm::eulerAngles(glm::quat(curTransform.q.w, curTransform.q.x, curTransform.q.y, curTransform.q.z));

		// FC, run current iteration:
		FCQuadState fcState;
		fcState.DeltaTime = DeltaTime;
		fcState.Height = mQuadTarget->Position.y;
		fcState.Pitch = mQuadTarget->Orientation.x;
		fcState.Yaw = mQuadTarget->Orientation.y;
		fcState.Roll = mQuadTarget->Orientation.z;
		FCCommands fcCommands = mFlightController->Iterate(fcState);

		// Apply quad commands
		glm::mat4 quadToWorld;
		quadToWorld = glm::rotate(quadToWorld, mQuadTarget->Orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		quadToWorld = glm::rotate(quadToWorld, mQuadTarget->Orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		quadToWorld = glm::rotate(quadToWorld, mQuadTarget->Orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
		
		// Thrust per motor:
		float flThrust = glm::clamp(fcCommands.FrontLeftThr, 0.0f, 1.0f) * (1.5f + rnd1);
		PxRigidBodyExt::addLocalForceAtLocalPos(*rigidBody, PxVec3(0.0f, flThrust, 0.0f), PxVec3(-dimX, 0.0f, dimZ));

		float rlThrust = glm::clamp(fcCommands.RearLeftThr,  0.0f, 1.0f) * (1.5f + rnd2);
		PxRigidBodyExt::addLocalForceAtLocalPos(*rigidBody, PxVec3(0.0f, rlThrust, 0.0f), PxVec3(-dimX, 0.0f, -dimZ));

		float frThrust = glm::clamp(fcCommands.FrontRightThr,0.0f, 1.0f) * (1.5f + rnd3);
		PxRigidBodyExt::addLocalForceAtLocalPos(*rigidBody, PxVec3(0.0f, frThrust, 0.0f), PxVec3( dimX, 0.0f, dimZ));

		float rrThrust = glm::clamp(fcCommands.RearRightThr, 0.0f, 1.0f) * (1.5f + rnd4);
		PxRigidBodyExt::addLocalForceAtLocalPos(*rigidBody, PxVec3(0.0f, rrThrust, 0.0f), PxVec3( dimX, 0.0f,-dimZ));

		// Local frame to world frame:
		//glm::vec3 localForce = glm::vec3(0.0f, flThrust + rlThrust + frThrust + rrThrust, 0.0f);
		//glm::vec3 worldForce =  glm::mat3(quadToWorld) * localForce;

		// Query sim state, used for the 3D visualization:
		SimulationFrame& frame = mResult.Frames[frameIdx];
		frame.QuadOrientation = mQuadTarget->Orientation;
		frame.QuadPosition = mQuadTarget->Position;
		
		frame.HeightPIDState.P = mFlightController->HeightPID.LastP;
		frame.HeightPIDState.I = mFlightController->HeightPID.LastI;
		frame.HeightPIDState.D = mFlightController->HeightPID.LastD;

		frame.PitchPIDState.P = mFlightController->PitchPID.LastP;
		frame.PitchPIDState.I = mFlightController->PitchPID.LastI;
		frame.PitchPIDState.D = mFlightController->PitchPID.LastD;

		frame.RollPIDState.P = mFlightController->RollPID.LastP;
		frame.RollPIDState.I = mFlightController->RollPID.LastI;
		frame.RollPIDState.D = mFlightController->RollPID.LastD;

		//frame.WorldForce = worldForce;

		// Step the physx simulation:
		physxScene->simulate(DeltaTime);
		physxScene->fetchResults(true);

		curTime += DeltaTime;
	}

	// Cleanup:
	rigidBody->release();
	physxScene->release();
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

const SimulationResult& Simulation::GetSimulationResults() const
{
	return mResult;
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

void SimulationResult::Reset()
{
	DeltaTime = 0.0f;
	Frames.clear();
}

const SimulationFrame::PIDState& SimulationFrame::GetPIDState(PIDType type)const
{
	switch (type)
	{
		case SimulationFrame::Height:	return HeightPIDState;
		case SimulationFrame::Pitch:	return PitchPIDState;
		case SimulationFrame::Roll:		return RollPIDState;
		default: assert(false);			return HeightPIDState;
	}
}
