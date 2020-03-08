#pragma once

class Quad;

class Simulation
{
public:
	Simulation();
	void SetQuadTarget(Quad* quad);
	void RenderUI();

	float TotalSimTime;
	float DeltaTime;

private:
	Quad* mQuadTarget;
};