#pragma once
#pragma warning (disable: 26495)

#include "Component.h"

#include "GameObject/GameObject.h"

#include "Math/float3.h"

#define COMPONENT_LIGHT "Light"

enum class LightType { UNKNOWN, DIRECTIONAL, POINT, SPOT, AMBIENT };

const static std::string GetNameByLightType(LightType type);
const static LightType GetLightTypeByName(const std::string& name);

class Json;

class ComponentLight : public Component
{
public:
	ComponentLight(const bool active, const std::shared_ptr<GameObject>& owner);
	ComponentLight(LightType type, bool canBeRemoved);
	ComponentLight(LightType type, const std::shared_ptr<GameObject>& gameObject, bool canBeRemoved);
	ComponentLight(LightType type, const float3& color, float intensity, bool canBeRemoved);
	ComponentLight(LightType type, const float3& color, float intensity, 
				   const std::shared_ptr<GameObject>& gameObject, bool canBeRemoved);

	virtual ~ComponentLight() {};

	void Update() override;

	void Enable() override;
	void Disable() override;

	virtual void Display() override;
	virtual void Draw() override {};

	virtual void SaveOptions(Json& meta) override {};
	virtual void LoadOptions(Json& meta) override {};

	const float3& GetColor() const;
	float GetIntensity() const;
	LightType GetLightType() const;

	void SetColor(const float3& color);
	void SetIntensity(float intensity);

protected:
	float3 color = float3(1.0f, 1.0f, 1.0f);
	float intensity = 1.f;

	LightType lightType;
};

inline void ComponentLight::Update()
{
	Draw();
}

inline void ComponentLight::Enable()
{
	Component::Enable();
}

inline void ComponentLight::Disable()
{
	Component::Disable();
}

const std::string GetNameByLightType(LightType type)
{
	switch (type)
	{
	case LightType::DIRECTIONAL:
		return "LightType_Directional";
		break;
	case LightType::POINT:
		return "LightType_Point";
		break;
	case LightType::SPOT:
		return "LightType_Spot";
		break;
	case LightType::AMBIENT:
		return "LightType_Ambient";
		break;
	default:
		assert(false && "Wrong light type introduced");
		return "";
	}
}

const LightType GetLightTypeByName(const std::string& typeName)
{
	if (typeName == "LightType_Directional")
		return LightType::DIRECTIONAL;
	if (typeName == "LightType_Point")
		return LightType::POINT;
	if (typeName == "LightType_Spot")
		return LightType::SPOT;
	if (typeName == "LightType_Ambient")
		return LightType::AMBIENT;
	return LightType::UNKNOWN;
}

inline const float3& ComponentLight::GetColor() const
{
	return color;
}

inline float ComponentLight::GetIntensity() const
{
	return intensity;
}

inline LightType ComponentLight::GetLightType() const
{
	return lightType;
}

inline void ComponentLight::SetColor(const float3& color)
{
	this->color = color;
}

inline void ComponentLight::SetIntensity(float intensity)
{
	this->intensity = intensity;
}