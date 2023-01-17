#include "ComponentLight.h"

#include "imgui.h"

ComponentLight::ComponentLight(const bool active, const std::shared_ptr<GameObject>& owner)
	: Component(ComponentType::LIGHT, active, owner, true)
{
}

ComponentLight::ComponentLight(LightType type, bool canBeRemoved) : Component(ComponentType::LIGHT, true, nullptr, canBeRemoved)
{
	this->lightType = type;
};

ComponentLight::ComponentLight(LightType type, const std::shared_ptr<GameObject>& gameObject, bool canBeRemoved) :
	Component(ComponentType::LIGHT, true, gameObject, canBeRemoved)
{
	this->lightType = type;
}

ComponentLight::ComponentLight(LightType type, const float3& color, float intensity, bool canBeRemoved) :
	Component(ComponentType::LIGHT, true, nullptr, canBeRemoved)
{
	this->lightType = type;
	this->color = color;
	this->intensity = intensity;
}

ComponentLight::ComponentLight(LightType type, const float3& color, float intensity,
							   const std::shared_ptr<GameObject>& gameObject, bool canBeRemoved) :
	Component(ComponentType::LIGHT, true, gameObject, canBeRemoved)
{
	this->lightType = type;
	this->color = color;
	this->intensity = intensity;
	
}

void ComponentLight::Display()
{
	if (ImGui::CollapsingHeader("BASIC LIGHT", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("This is a basic light");
	}

	ImGui::Separator();
}