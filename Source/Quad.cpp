#include "Quad.h"

#include "Graphics/UI/IMGUI/imgui.h"

Quad::Quad():
	 Mass(0.081f)
	,Width(0.16f)
	,Height(0.05f)
	,Depth(0.16f)
{
}

void Quad::RenderUI()
{
	ImGui::InputFloat("Mass", &Mass);
	ImGui::InputFloat("Width", &Width);
	ImGui::InputFloat("Height", &Height);
	ImGui::InputFloat("Depth", &Depth);
}

void Quad::Reset()
{
	Position = glm::vec3(0.0f);
	Orientation = glm::vec3(0.0f);
}
