#pragma once

#include "ComponentWindow.h"
#include "Math/float4.h"

class ComponentMeshRenderer;
class WindowMeshInput;
class WindowTextureInput;
class WindowMaterialInput;
class ResourceTexture;
class ResourceMaterial;

class WindowComponentMeshRenderer : public ComponentWindow
{
public:
	WindowComponentMeshRenderer(ComponentMeshRenderer* component);
	~WindowComponentMeshRenderer() override;

	void SetMaterial(const std::shared_ptr<ResourceMaterial>& material);
	void SetDiffuse(const std::shared_ptr<ResourceTexture>& diffuseTexture);
	void SetNormal(const std::shared_ptr<ResourceTexture>& normalMap);
	void SetMetallic(const std::shared_ptr<ResourceTexture>& metallicMap);
	void SetSpecular(const std::shared_ptr<ResourceTexture>& specularMap);
	void SetEmission(const std::shared_ptr<ResourceTexture>& emissionMap);

protected:
	void DrawWindowContents() override;

private:
	void DrawSetMaterial();
	void DrawEmptyMaterial();
	void InitMaterialValues();

	float4 colorDiffuse;
	float3 colorSpecular;
	float2 tiling;
	float2 offset;
	std::shared_ptr<ResourceMaterial> material;
	std::shared_ptr<ResourceTexture> diffuseTexture;
	std::shared_ptr<ResourceTexture> normalMap;
	std::shared_ptr<ResourceTexture> metallicMap;
	std::shared_ptr<ResourceTexture> specularMap;
	std::shared_ptr<ResourceTexture> emissionMap;

	float smoothness;
	float metalness;
	float normalStrength;
	bool isTransparent;
	unsigned int currentShaderTypeIndex;
	unsigned int currentTransparentIndex;
	static const std::vector<std::string> shaderTypes;
	static const std::vector<std::string> renderModes;

	std::unique_ptr<WindowMeshInput> inputMesh;
	std::unique_ptr<WindowMaterialInput> inputMaterial;
	std::unique_ptr<WindowTextureInput> inputTextureDiffuse;
	std::unique_ptr<WindowTextureInput> inputTextureNormal;
	std::unique_ptr<WindowTextureInput> inputTextureMetallic;
	std::unique_ptr<WindowTextureInput> inputTextureSpecular;
	std::unique_ptr<WindowTextureInput> inputTextureEmission;

	ComponentMeshRenderer* oldComponent;

	bool reset;
	bool newMaterial;
	bool changeBatch;
};

inline void WindowComponentMeshRenderer::SetMaterial(const std::shared_ptr<ResourceMaterial>& material)
{
	this->material = material;
	newMaterial = true;
}

inline void WindowComponentMeshRenderer::SetDiffuse(const std::shared_ptr<ResourceTexture>& diffuseTexture)
{
	this->diffuseTexture = diffuseTexture;
}

inline void WindowComponentMeshRenderer::SetNormal(const std::shared_ptr<ResourceTexture>& normalMap)
{
	this->normalMap = normalMap;
}

inline void WindowComponentMeshRenderer::SetMetallic(const std::shared_ptr<ResourceTexture>& metallicMap)
{
	this->metallicMap = metallicMap;
}

inline void WindowComponentMeshRenderer::SetSpecular(const std::shared_ptr<ResourceTexture>& specularMap)
{
	this->specularMap = specularMap;
}

inline void WindowComponentMeshRenderer::SetEmission(const std::shared_ptr<ResourceTexture>& emissionMap)
{
	this->emissionMap = emissionMap;
}
