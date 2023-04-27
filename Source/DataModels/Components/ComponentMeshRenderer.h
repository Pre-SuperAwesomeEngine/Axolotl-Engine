#pragma once
#include "Component.h"
#include "Auxiliar/Generics/Drawable.h"

#include "Globals.h"

#include "Components/Component.h"

#include "FileSystem/UniqueID.h"
#include "Batch/GeometryBatch.h"


#include <memory>

#include "FileSystem/UniqueID.h"

#include "Math/float3.h"
#include "Math/float4.h"

class ResourceMesh;
class ResourceMaterial;
class ResourceTexture;
class Json;
class WindowMeshInput;
class WindowMaterialInput;
class WindowTextureInput;

class ComponentMeshRenderer : public Component, public Drawable
{
public:
	ComponentMeshRenderer(const bool active, GameObject* owner);
	ComponentMeshRenderer(const ComponentMeshRenderer& componentMeshRenderer);
	~ComponentMeshRenderer() override;

	void Draw() const override;
	void DrawMeshes(Program* program) const;
	void DrawMaterial(Program* program) const;
	void DrawHighlight() const;

	void SaveOptions(Json& meta) override;
	/*void SaveUIDOfResourceToMeta
		(Json& meta, const char* field, const ResourceTexture* texturePtr);*/
	void LoadOptions(Json& meta) override;

	void SetMesh(const std::shared_ptr<ResourceMesh>& newMesh);
	void SetMaterial(const std::shared_ptr<ResourceMaterial>& newMaterial);

	// Common attributes (setters)
	void SetDiffuseColor(float4& diffuseColor);
	void SetSmoothness(float smoothness);
	void SetNormalStrenght(float normalStrength);

	// Default shader attributes (setters)
	void SetMetalness(float metalness);

	// Specular shader attributes (setters)
	void SetSpecularColor(float3& specularColor);
	
	std::shared_ptr<ResourceMesh> GetMesh() const;
	std::shared_ptr<ResourceMaterial> GetMaterial() const;

	GeometryBatch* GetBatch() const;
	void SetBatch(GeometryBatch* geometryBatch);

	// Common attributes (getters)
	const float4& GetDiffuseColor() const;
	const float GetSmoothness() const;
	const float GetNormalStrenght() const;

	// Default shader attributes (getters)
	const float GetMetalness() const;

	// Specular shader attributes (getters)
	const float3& GetSpecularColor() const;

	void UnloadTextures();
	void UnloadTexture(TextureType textureType);

private:
	bool IsMeshLoaded() const;
	bool IsMaterialLoaded() const;

	mutable std::shared_ptr<ResourceMesh> mesh;
	mutable std::shared_ptr<ResourceMaterial> material;



	const unsigned int GetShaderType() const;

	GeometryBatch* batch;
	WindowMeshInput* inputMesh;
	WindowMaterialInput* inputMaterial;

};

inline std::shared_ptr<ResourceMesh> ComponentMeshRenderer::GetMesh() const
{
	return mesh;
}

inline std::shared_ptr<ResourceMaterial> ComponentMeshRenderer::GetMaterial() const
{
	return material;
}

inline GeometryBatch* ComponentMeshRenderer::GetBatch() const
{
	return batch;
}

inline void ComponentMeshRenderer::SetBatch(GeometryBatch* geometryBatch)
{
	batch = geometryBatch;
}

inline bool ComponentMeshRenderer::IsMeshLoaded() const
{
	return mesh != nullptr;
}

inline bool ComponentMeshRenderer::IsMaterialLoaded() const
{
	return material != nullptr;
}