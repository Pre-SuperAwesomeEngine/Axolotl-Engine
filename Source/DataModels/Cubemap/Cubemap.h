#pragma once
#include "GL/glew.h"
#include <memory>

class ResourceTexture;

class Cubemap
{
public:

	Cubemap();
	~Cubemap();
	void RenderCube();

private:


	GLuint frameBuffer;
	GLuint renderBuffer;
	GLuint enviromentTexture;
	unsigned int cubeVAO;
	unsigned int cubeVBO;

	std::shared_ptr<ResourceTexture> hdrTexture;
};

