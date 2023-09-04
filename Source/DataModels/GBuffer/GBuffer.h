#pragma once

#include "GL/glew.h"

class GBuffer
{

public :

	GBuffer();
	~GBuffer();

	void BindFrameBuffer();
	void ReadFrameBuffer();
	void DrawFrameBuffer();
	void UnBindFrameBuffer();

	void BindTexture();
	void UnBindTexture();

	GLuint GetEmissiveTexture();

	void InitGBuffer(unsigned width, unsigned height);

	GLuint GetDepthTexture();

private :

	GLuint gFrameBuffer;
	GLuint gPosition;
	GLuint gNormal;
	GLuint gDiffuse;
	GLuint gSpecular;
	GLuint gEmissive;
	GLuint gDepth;
};

inline GLuint GBuffer::GetDepthTexture()
{
	return gDepth;
}

inline GLuint GBuffer::GetEmissiveTexture()
{
	return gEmissive;
}

