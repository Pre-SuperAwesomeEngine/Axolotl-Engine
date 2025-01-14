#pragma once

#include "ParticleModule.h"

#define MAX_SPAWN_RATE 1000

#define DEFAULT_SPAWN_RATE 5.0f

class ModuleSpawn : public ParticleModule
{
public:
	ModuleSpawn(ParticleEmitter* emitter);
	ModuleSpawn(ParticleEmitter* emitter, ModuleSpawn* spawn);
	~ModuleSpawn() override;

	void Spawn(EmitterInstance* instance) override;
	void Update(EmitterInstance* instance) override;
	void CopyConfig(ParticleModule* module) override;
	void DrawImGui() override;

	void SetSpawnRate(float spawnRate);

	float GetSpawnRate() const;

private:
	float spawnRate;
};

inline void ModuleSpawn::SetSpawnRate(float spawnRate)
{
	this->spawnRate = spawnRate;
}

inline float ModuleSpawn::GetSpawnRate() const
{
	return spawnRate;
}
