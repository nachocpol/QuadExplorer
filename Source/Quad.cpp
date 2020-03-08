#include "Quad.h"

#include "Graphics/UI/IMGUI/imgui.h"

Quad::Quad():
	 Mass(0.2f)
	,Width(0.25f)
	,Height(0.05f)
	,Depth(0.25f)
{
}

void Quad::RenderUI()
{
	ImGui::Begin("Quad");
	ImGui::InputFloat("Mass", &Mass);
	ImGui::End();
}

void Quad::Reset()
{
	Position = glm::vec3(0.0f);
	Orientation = glm::quat();
}
