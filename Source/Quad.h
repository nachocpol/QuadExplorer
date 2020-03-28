#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

class Quad
{
public:
	Quad();
	void RenderUI();
	void Reset();

	float Mass;
	float Width;
	float Height;
	float Depth;

	// The simulation will drive this values
	glm::vec3 Position;
	glm::vec3 Orientation;
};