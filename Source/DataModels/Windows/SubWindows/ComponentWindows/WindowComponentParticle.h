#pragma once

#include "ComponentWindow.h"

#include "Components/ComponentParticleSystem.h"

class WindowParticleSystemInput;

class WindowComponentParticle : public ComponentWindow
{
public:
	WindowComponentParticle(ComponentParticleSystem* component);
	~WindowComponentParticle() override;

protected:
	void DrawWindowContents() override;

private:
	void DrawEmitter(EmitterInstance* instance);
	std::unique_ptr<WindowParticleSystemInput> inputParticleSystem;
};

