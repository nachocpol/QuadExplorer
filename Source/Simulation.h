#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

#include <vector>

class Quad;
class BaseFlyController;

struct SimulationFrame
{
	struct PIDState
	{
		float P;
		float I;
		float D;
	};
	enum PIDType
	{
		Height,
		Pitch,
		Roll
	};
	const PIDState& GetPIDState(PIDType type)const;
	static SimulationFrame Interpolate(const SimulationFrame& a, const SimulationFrame& b, float alpha);
	glm::vec3 QuadPosition;
	glm::vec3 QuadOrientation;
	glm::vec3 WorldForce;
	PIDState HeightPIDState;
	PIDState PitchPIDState;
	PIDState RollPIDState;
};

struct SimulationResult
{
	void Reset();
	float DeltaTime;
	std::vector<SimulationFrame> Frames;
};

class Simulation
{
public:
	Simulation();
	void Init();
	void SetQuadTarget(Quad* quad);
	void SetFlightController(BaseFlyController* fc);
	void RenderUI();
	void RunSimulation();
	SimulationFrame GetSimulationFrame(float simTime, bool interpolate = true);
	SimulationFrame GetSimulationFrameFromIdx(int index);
	const SimulationResult& GetSimulationResults()const;
	bool HasResults()const;
	int GetNumFrames();

	float TotalSimTime;
	float DeltaTime;

private:
	SimulationResult mResult;
	Quad* mQuadTarget;
	BaseFlyController* mFlightController;
};