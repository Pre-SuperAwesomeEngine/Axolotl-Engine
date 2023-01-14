#pragma once

#include "ComponentLight.h"

class ComponentPointLight : public ComponentLight
{
public:
	ComponentPointLight();
	ComponentPointLight(GameObject* parent);
	ComponentPointLight(float radius, const float3& color, float intensity);
	ComponentPointLight(float radius, const float3& color, float intensity, GameObject* parent);
	~ComponentPointLight() {};

	void Draw() override;
	void Display() override;

	float GetRadius() const;

	void SetRadius(float radius);

private:
	float radius = 1.0f;
};

inline float ComponentPointLight::GetRadius() const
{
	return radius;
}

inline void ComponentPointLight::SetRadius(float radius)
{
	this->radius = radius;
}
