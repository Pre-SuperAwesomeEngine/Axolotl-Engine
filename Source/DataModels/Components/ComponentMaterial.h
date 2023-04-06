#pragma once
#pragma warning (disable: 26495)

#include "Components/Component.h"

#include "Math/float3.h"

#include "FileSystem/UniqueID.h"
#include "Globals.h"



class ResourceMaterial;

class Json;

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(bool active, GameObject* owner);
	ComponentMaterial(const ComponentMaterial& componentMaterial);
	~ComponentMaterial() override;

	void Update() override;

	void Draw() override;

	void SaveOptions(Json& meta) override;
	void SaveUIDOfResourceToMeta(Json& meta, const char* field, const ResourceTexture* texturePtr);
	void LoadOptions(Json& meta) override;

	void SetMaterial(const std::shared_ptr<ResourceMaterial>& newMaterial);
	void SetDiffuseColor(float3& diffuseColor);
	void SetSpecularColor(float3& specularColor);
	void SetShininess(float shininess);
	void SetNormalStrenght(float normalStrength);
	void SetSmoothness(float smoothness);
	void SetMetalness(float metalness);
	void SetHasShininessAlpha(bool hasShininessAlpha);
	void SetMetallicAlpha(bool metallicAlpha);

	std::shared_ptr<ResourceMaterial> GetMaterial() const;
	const float3& GetDiffuseColor() const;
	const float3& GetSpecularColor() const;
	//const float GetShininess() const;
	const float GetNormalStrenght() const;
	const float GetSmoothness() const;
	const float GetMetalness() const;
	//const bool HasShininessAlpha() const;
	const bool HasMetallicAlpha() const;

private:

	void UnloadTextures();
	void UnloadTexture(TextureType textureType);

	
};
