#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include <vector>

class Quad;
class QuadFlyController;

struct SimulationFrame
{
	glm::vec3 QuadPosition;
	glm::quat QuadOrientation;
};

struct SimulationResult
{
	float DeltaTime;
	std::vector<SimulationFrame> Frames;
};

class Simulation
{
public:
	Simulation();
	void Init();
	void SetQuadTarget(Quad* quad);
	void SetFlightController(QuadFlyController* fc);
	void RenderUI();
	void RunSimulation();
	SimulationFrame GetSimulationFrame(float simTime);
	SimulationFrame GetSimulationFrameFromIdx(int index);
	bool HasResults()const;
	int GetNumFrames();

	float TotalSimTime;
	float DeltaTime;

private:
	SimulationResult mResult;
	Quad* mQuadTarget;
	QuadFlyController* mFlightController;
};