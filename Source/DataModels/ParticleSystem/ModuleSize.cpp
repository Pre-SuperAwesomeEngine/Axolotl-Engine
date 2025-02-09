#include "StdAfx.h"

#include "ModuleSize.h"

#include "EmitterInstance.h"

#include "ImGui/imgui.h"

ModuleSize::ModuleSize(ParticleEmitter* emitter) :
	ParticleModule(ModuleType::SIZE, emitter),
	random(true), sizeOverTime(float2(0.001f, 0.01f))
{
}

ModuleSize::ModuleSize(ParticleEmitter* emitter, ModuleSize* size) :
	ParticleModule(ModuleType::SIZE, emitter)
{
	random = size->IsRandom();
	sizeOverTime = size->GetSize();
	enabled = size->IsEnabled();
}

ModuleSize::~ModuleSize()
{
}

void ModuleSize::Spawn(EmitterInstance* instance)
{
}

void ModuleSize::Update(EmitterInstance* instance)
{
	if (enabled)
	{
		std::vector<EmitterInstance::Particle>& particles = instance->GetParticles();

		for (int i = 0; i < particles.size(); ++i)
		{
			EmitterInstance::Particle& particle = particles[i];

			if (!particle.dead)
			{
				if (particle.sizeOverTime == -1.0f)
				{
					particle.sizeOverTime = random ? 
						instance->CalculateRandomValueInRange(sizeOverTime.x, sizeOverTime.y) : sizeOverTime.x;
				}

				particle.size += particle.sizeOverTime;
				if (particle.size < 0.0f)
				{
					particle.size = 0.0f;
				}
			}
		}
	}
}

void ModuleSize::CopyConfig(ParticleModule* module)
{
	ModuleSize* size = static_cast<ModuleSize*>(module);

	enabled = size->IsEnabled();
	random = size->IsRandom();
	sizeOverTime = size->GetSize();
}

void ModuleSize::DrawImGui()
{
	if (ImGui::TreeNodeEx("Size Module", ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_FramePadding))
	{
		ImGui::SameLine();
		ImGui::Checkbox("##enabled3", &enabled);

		ImGui::Text("Size over time");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(165.0f);
		if (random)
		{
			if (ImGui::DragFloat2("##sliderSizeOverTime", &sizeOverTime[0], 1.0f, 
				MIN_SIZE_OVER_TIME, MAX_SIZE_OVER_TIME, "%.3f"))
			{
				if (sizeOverTime.x > sizeOverTime.y)
				{
					sizeOverTime.x = sizeOverTime.y;
				}
				else if (sizeOverTime.x < MIN_SIZE_OVER_TIME)
				{
					sizeOverTime.x = MIN_SIZE_OVER_TIME;
				}

				if (sizeOverTime.y < sizeOverTime.x)
				{
					sizeOverTime.y = sizeOverTime.x;
				}
				else if (sizeOverTime.y > MAX_SIZE_OVER_TIME)
				{
					sizeOverTime.y = MAX_SIZE_OVER_TIME;
				}
			}
		}
		else
		{
			if (ImGui::InputFloat("##inputSizeOverTime", &sizeOverTime.x, 0.001f, 0.01f, "%.3f"))
			{
				if (sizeOverTime.x > MAX_SIZE_OVER_TIME)
				{
					sizeOverTime.x = MAX_SIZE_OVER_TIME;
				}
				else if (sizeOverTime.x < MIN_SIZE_OVER_TIME)
				{
					sizeOverTime.x = MIN_SIZE_OVER_TIME;
				}
			}
		}
		ImGui::SameLine(0.0f, 5.0f);
		ImGui::Text("Random");
		ImGui::SameLine(0.0f, 5.0f);
		ImGui::Checkbox("##randomSizeOverTime", &random);

		ImGui::TreePop();
	}
	else
	{
		ImGui::SameLine();
		ImGui::Checkbox("##enabled3", &enabled);
	}
}
