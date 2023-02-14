#pragma once
#pragma warning (disable: 26495)

#include "Module.h"
#include "DataStructures/Quadtree.h"
#include "GL/glew.h"

struct SDL_Texture;
struct SDL_Renderer;
struct SDL_Rect;

class Skybox;

class ModuleRender : public Module
{
public:
	ModuleRender();
	~ModuleRender() override;

	bool Init() override;
	bool Start() override;
	bool CleanUp() override;

	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();

	void WindowResized(unsigned width, unsigned height);
	void UpdateBuffers(unsigned width, unsigned height);

	void SetShaders(const std::string& vertexShader, const std::string& fragmentShader);

	void SetBackgroundColor(float4 color);
	float4 GetBackgroundColor() const;

	unsigned int GetRenderedTexture() const;
	const std::string& GetVertexShader() const;
	const std::string& GetFragmentShader() const;

	void FillRenderList(const Quadtree* quadtree);
	void AddToRenderList(const std::shared_ptr<GameObject>& gameObject);

	//void DrawScene(Quadtree* quadtree);

	bool IsSupportedPath(const std::string& modelPath);
	void DrawQuadtree(const Quadtree* quadtree);

private:
	void UpdateProgram();

	void* context;
	float4 backgroundColor;

	unsigned vbo;
	
	std::vector<std::weak_ptr<GameObject> > gameObjectsToDraw;
	const std::vector<std::string> modelTypes = { "FBX" };

	//should this be here?
	std::unique_ptr<Skybox> skybox;

	GLuint frameBuffer = 0;
	GLuint renderedTexture = 0;
	GLuint depthRenderBuffer = 0;

	std::string vertexShader = "default_vertex.glsl";
	std::string fragmentShader = "default_fragment.glsl";

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

inline unsigned int ModuleRender::GetRenderedTexture() const
{
	return renderedTexture;
}

inline const std::string& ModuleRender::GetVertexShader() const
{
	return vertexShader;
}

inline const std::string& ModuleRender::GetFragmentShader() const
{
	return fragmentShader;
}

