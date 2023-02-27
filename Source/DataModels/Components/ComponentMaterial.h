#pragma once
#pragma warning (disable: 26495)

#include "Components/Component.h"

#include "Math/float3.h"

#include "FileSystem/UniqueID.h"
#include "Globals.h"

#include <memory>

class WindowTextureInput;
class WindowMaterialInput;
class ResourceMaterial;
class ResourceTexture;
class Json;

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(bool active, const std::shared_ptr<GameObject>& owner);
	~ComponentMaterial() override;

	void Update() override;

	void Draw() override;

	void SaveOptions(Json& meta) override;
	void SaveUIDOfResourceToMeta(Json& meta, const char* field, const std::weak_ptr<ResourceTexture>& texturePtr);
	void LoadOptions(Json& meta) override;

	void SetMaterial(const std::shared_ptr<ResourceMaterial>& newMaterial);
	void SetDiffuseColor(float3& diffuseColor);
	void SetSpecularColor(float3& specularColor);
	void SetShininess(float shininess);
	void SetNormalStrenght(float normalStrength);
	void SetHasShininessAlpha(bool hasShininessAlpha);

	std::weak_ptr<ResourceMaterial> GetMaterial() const;
	const float3& GetDiffuseColor() const;
	const float3& GetSpecularColor() const;
	const float GetShininess() const;
	const float GetNormalStrenght() const;
	const bool HasShininessAlpha() const;

private:

	void UnloadTextures();
	void UnloadTexture(TextureType textureType);

	std::shared_ptr<ResourceMaterial> material;

	float3 diffuseColor = float3(1.0, 1.0, 0.0);
	float3 specularColor = float3(0.5, 0.5, 0.5);
	float shininess = 512.f;
	float normalStrength = 1.0f;

	bool hasShininessAlpha = false;

	friend class WindowComponentMaterial;
};

inline void ComponentMaterial::SetDiffuseColor(float3& diffuseColor)
{
	this->diffuseColor = diffuseColor;
}

inline void ComponentMaterial::SetSpecularColor(float3& specularColor)
{
	this->specularColor = specularColor;
}

inline void ComponentMaterial::SetShininess(float shininess)
{
	this->shininess = shininess;
}

inline void ComponentMaterial::SetNormalStrenght(float normalStrength)
{
	this->normalStrength = normalStrength;
}

inline void ComponentMaterial::SetHasShininessAlpha(bool hasShininessAlpha)
{
	this->hasShininessAlpha = hasShininessAlpha;
}

inline std::weak_ptr<ResourceMaterial> ComponentMaterial::GetMaterial() const
{
	return material;
}

inline const float3& ComponentMaterial::GetDiffuseColor() const {
	return diffuseColor;
}

inline const float3& ComponentMaterial::GetSpecularColor() const {
	return specularColor;
}

inline const float ComponentMaterial::GetShininess() const {
	return shininess;
}

inline const float ComponentMaterial::GetNormalStrenght() const {
	return normalStrength;
}

inline const bool ComponentMaterial::HasShininessAlpha() const {
	return hasShininessAlpha;
}