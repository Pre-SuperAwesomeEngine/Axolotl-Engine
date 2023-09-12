#pragma once

#include "Module.h"

#include "GL/glew.h"
#include "Math/float4.h"

#include "FileSystem/UID.h"

#include "PostProcess/SSAO.h"

#include "Render/Shadows.h"

#define KAWASE_DUAL_SAMPLERS 4
#define GAUSSIAN_BLUR_SHADOW_MAP 2

struct SDL_Texture;
struct SDL_Renderer;
struct SDL_Rect;

class Quadtree;
class Program;
class Cubemap;
class GameObject;
class Camera;
class GeometryBatch;
class BatchManager;
class ComponentMeshRenderer;
class GBuffer;

class ModuleRender : public Module
{
public:

	ModuleRender();
	~ModuleRender() override;

	bool Init() override;

	UpdateStatus PreUpdate() override;
	UpdateStatus Update() override;
	UpdateStatus PostUpdate() override;

	bool CleanUp() override;

	void WindowResized(unsigned width, unsigned height);
	void UpdateBuffers(unsigned width, unsigned height);

	void SetBackgroundColor(float4 color);
	float4 GetBackgroundColor() const;

	void ChangeRenderMode();
	void ChangeToneMapping();
	void SwitchBloomActivation();
	void ToggleShadows();
	void ToggleVSM();
	void ToggleSSAO();
	void ToggleCSMDebug();

	GLuint GetCameraUBO() const;
	GLuint GetRenderedTexture() const;
	float GetObjectDistance(const GameObject* gameObject);
	Shadows* GetShadows() const;

	void SetBloomIntensity(float color);
	float GetBloomIntensity() const;

	BatchManager* GetBatchManager() const;

	void FillRenderList(const Quadtree* quadtree, Camera* camera);
	void AddToRenderList(const GameObject* gameObject, Camera* camera, bool recursive = false);

	bool IsObjectInsideFrustrum(const GameObject* gameObject);

	void DrawQuadtree(const Quadtree* quadtree);

	void FillCharactersBatches();
	void RelocateGOInBatches(GameObject* go);

private:

	enum class ModeRender {
		DEFAULT = 0,
		POSITION = 1,
		NORMAL = 2,
		DIFFUSE = 3,
		SPECULAR = 4,
		EMISSIVE = 5,
		LENGTH
	};

	enum class ToneMappingMode {
		NONE = 0,
		UNCHARTED2 = 1,
		ACES_FILM = 2,
		LENGTH
	};

	bool CheckIfTransparent(const GameObject* gameObject);

	void DrawHighlight(GameObject* gameObject);

	void BindCameraToProgram(Program* program, Camera* camera);
	void BindCubemapToProgram(Program* program);

	void KawaseDualFiltering();

	Camera* GetFrustumCheckedCamera() const;

private:

	void* context;

	float4 backgroundColor;

	float4x4 dirLightView;
	float4x4 dirLightProj;

	BatchManager* batchManager;
	GBuffer* gBuffer;
	Shadows* shadows;
	SSAO* ssao;

	unsigned uboCamera;

	unsigned modeRender;
	unsigned toneMappingMode;
	unsigned bloomActivation;
	float bloomIntensity;
	float threshold;
	
	std::unordered_set<const GameObject*> gameObjectsInFrustrum;
	std::unordered_map<const GameObject*, float> objectsInFrustrumDistances;

	// 0: used in game and engine 
	// 1: only in engine, stores the final result, to avoid writing and reading at the same time
	GLuint frameBuffer[2] = {0, 0};
	GLuint renderedTexture[2] = {0, 0};

	GLuint dualKawaseDownFramebuffers[KAWASE_DUAL_SAMPLERS];
	GLuint dualKawaseDownTextures[KAWASE_DUAL_SAMPLERS];
	GLuint dualKawaseUpFramebuffers[KAWASE_DUAL_SAMPLERS];
	GLuint dualKawaseUpTextures[KAWASE_DUAL_SAMPLERS];

	GLuint depthStencilRenderBuffer = 0;
	
	//GLuint bloomFramebuffer;
	//GLuint bloomTexture;
	

	friend class ModuleEditor;
};

inline void ModuleRender::SetBackgroundColor(float4 color)
{
	backgroundColor = color;
}

inline float4 ModuleRender::GetBackgroundColor() const
{
	return backgroundColor;
}

inline void ModuleRender::ChangeRenderMode()
{
	modeRender = (modeRender + 1) % static_cast<int>(ModeRender::LENGTH);
}

inline void ModuleRender::ChangeToneMapping()
{
	toneMappingMode = (toneMappingMode + 1) % static_cast<int>(ToneMappingMode::LENGTH);
}

inline void ModuleRender::SwitchBloomActivation()
{
	bloomActivation = (bloomActivation + 1) % 2;
}

inline void ModuleRender::ToggleShadows()
{
	shadows->ToggleShadows();
}

inline void ModuleRender::ToggleVSM()
{
	shadows->ToggleVSM();
}

inline void ModuleRender::ToggleSSAO()
{
	ssao->ToggleSSAO();
}

inline void ModuleRender::ToggleCSMDebug()
{
	shadows->ToggleCSMDebug();
}

inline GLuint ModuleRender::GetCameraUBO() const
{
	return uboCamera;
}

inline GLuint ModuleRender::GetRenderedTexture() const
{
	return renderedTexture[1];
}

inline BatchManager* ModuleRender::GetBatchManager() const
{
	return batchManager;
}

inline bool ModuleRender::IsObjectInsideFrustrum(const GameObject* gameObject)
{
	return gameObjectsInFrustrum.find(gameObject) != gameObjectsInFrustrum.end();
}

inline float ModuleRender::GetObjectDistance(const GameObject* gameObject)
{
	return objectsInFrustrumDistances[gameObject];
}

inline Shadows* ModuleRender::GetShadows() const
{
	return shadows;
}

inline void ModuleRender::SetBloomIntensity(float intensity)
{
	bloomIntensity = intensity;
}

inline float ModuleRender::GetBloomIntensity() const
{
	return bloomIntensity;
}
