#include "HealthSystem.h"

#include "Modules/ModuleScene.h"
#include "Modules/ModuleInput.h"
#include "Scene/Scene.h"

#include "Components/ComponentAnimation.h"
#include "Components/ComponentScript.h"
#include "Components/Component.h"

REGISTERCLASS(HealthSystem);

HealthSystem::HealthSystem() : Script(), currentHealth(100), maxHealth(100), componentAnimation(nullptr), loseSceneName("00_LoseScene_VS3"), dead(false)
{
	REGISTER_FIELD(currentHealth, float);
	REGISTER_FIELD(maxHealth, float);
	REGISTER_FIELD(loseSceneName, std::string);
}

void HealthSystem::Start()
{
	componentAnimation = static_cast<ComponentAnimation*>(owner->GetComponent(ComponentType::ANIMATION));

	if (maxHealth < currentHealth)
	{
		maxHealth = currentHealth;
	}
}

void HealthSystem::Update(float deltaTime)
{
	if (dead && owner->CompareTag("Player"))
	{
#ifndef ENGINE
		if (loseSceneName != "" && !componentAnimation->isPlaying() && componentAnimation->GetActualStateName() == "Death")
		{
			App->GetModule<ModuleScene>()->SetSceneToLoad("Lib/Scenes/" + loseSceneName + ".axolotl");
		}
#endif // ENGINE
		if(!componentAnimation->isPlaying() && componentAnimation->GetActualStateName() == "Death")
		{
			ENGINE_LOG("Player is dead");

			PlayerDeath();
		}
	}

	// Provisional here until we have a way to delay a call to a function a certain time
	// This should go inside the TakeDamage function but delay setting it to false by 2 seconds or smth like that
	
	if (currentHealth <= 0)
	{
		componentAnimation->SetParameter("IsDead", true);
		dead = true;
	}
	else 
	{
		componentAnimation->SetParameter("IsTakingDamage", false);
	}
}

void HealthSystem::TakeDamage(float damage)
{
	currentHealth -= damage;

	componentAnimation->SetParameter("IsTakingDamage", true);
}

void HealthSystem::HealLife(float amountHealed)
{
	currentHealth = std::min(currentHealth + amountHealed, maxHealth);
}

bool HealthSystem::EntityIsAlive() const
{
	return currentHealth > 0;
}

void HealthSystem::PlayerDeath()
{
	// Once the player is dead, disable its scripts
	std::vector<ComponentScript*> gameObjectScripts =
		owner->GetComponentsByType<ComponentScript>(ComponentType::SCRIPT);

	for (ComponentScript* script : gameObjectScripts)
	{
		script->Disable();
	}

	GameObject::GameObjectView children = owner->GetChildren();
	GameObject* cameraChild;

	for (const GameObject* child : children)
	{
		if (child->GetComponent(ComponentType::CAMERA))
		{
			std::vector<ComponentScript*> cameraScripts =
				child->GetComponentsByType<ComponentScript>(ComponentType::SCRIPT);

			for (ComponentScript* script : cameraScripts)
			{
				script->Disable();
			}
		}
	}
}