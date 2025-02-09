#include "StdAfx.h"
#include "PauseManager.h"

#include "EnemyClass.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentAnimation.h"

#include "PlayerManagerScript.h"
#include "SpaceshipMovement.h"
#include "RangedFastAttackBullet.h"
#include "BossChargeRockScript.h"
#include "LightAttackBullet.h"
#include "JumpFinisherAttackBullet.h"
#include "BossMissilesMissileScript.h"

#include "ModulePlayer.h"
#include "ModuleScene.h"
#include "Scene/Scene.h"
#include "Application.h"

REGISTERCLASS(PauseManager);

PauseManager::PauseManager() : Script(), enemiesGameObjects{}, enemyTag("Enemy"),
bulletTag("Bullet"), alluraBulletTag("AlluraBullet"), rockTag("Rock"),
missileTag("Missile"), bulletGameObjects{}, alluraBulletGameObjects{}, 
rockGameObjects{}, missileGameObjects{}
{
	REGISTER_FIELD(enemyTag, std::string);
	REGISTER_FIELD(alluraBulletTag, std::string);
	REGISTER_FIELD(rockTag, std::string);
	REGISTER_FIELD(bulletTag, std::string);
	REGISTER_FIELD(missileTag, std::string);
}

void PauseManager::Start()
{
	scene = App->GetModule<ModuleScene>()->GetLoadedScene();
	enemiesGameObjects = scene->SearchGameObjectByTag(enemyTag);
}

void PauseManager::Pause(bool paused)
{
	bulletGameObjects = scene->SearchGameObjectByTag(bulletTag);
	alluraBulletGameObjects = scene->SearchGameObjectByTag(alluraBulletTag);
	rockGameObjects = scene->SearchGameObjectByTag(rockTag);
	missileGameObjects = scene->SearchGameObjectByTag(missileTag);

	PauseEnemies(paused);
	PauseBullets(paused);
	PauseRocks(paused);
	PauseMissiles(paused);
	PausePlayer(paused);
}

void PauseManager::PausePlayer(bool paused)
{
	GameObject* player = App->GetModule<ModulePlayer>()->GetPlayer();
	if (player->HasComponent<PlayerManagerScript>())
	{
		player->GetComponent<PlayerManagerScript>()->FullPausePlayer(paused);
	}
	else
	{
		player->GetComponent<SpaceshipMovement>()->SetIsPaused(paused);
	}
}
	
void PauseManager::PauseEnemies(bool paused)
{
	for (int i = 0; i < enemiesGameObjects.size(); ++i)
	{
		enemiesGameObjects[i]->GetComponent<EnemyClass>()->PauseEnemy(paused);
		if (paused)
		{
			enemiesGameObjects[i]->GetComponent<ComponentAnimation>()->Disable();
		}
		else
		{
			enemiesGameObjects[i]->GetComponent<ComponentAnimation>()->Enable();
		}
	}
}

void PauseManager::PauseBullets(bool paused)
{
	for (int i = 0; i < bulletGameObjects.size(); ++i)
	{
		if (bulletGameObjects[i]->HasComponent<RangedFastAttackBullet>())
		{
			bulletGameObjects[i]->GetComponent<RangedFastAttackBullet>()->SetPauseBullet(paused);
		}
		else if (bulletGameObjects[i]->IsEnabled())
		{
			bulletGameObjects[i]->GetComponent<BossMissilesMissileScript>()->SetIsPaused(paused);
		}
	}
	
	for (int i = 0; i < alluraBulletGameObjects.size(); ++i)
	{
		if (alluraBulletGameObjects[i]->HasComponent<LightAttackBullet>())
		{
			alluraBulletGameObjects[i]->GetComponent<LightAttackBullet>()->SetPauseBullet(paused);
		}
		else if(alluraBulletGameObjects[i]->IsEnabled())
		{
			alluraBulletGameObjects[i]->GetComponent<JumpFinisherAttackBullet>()->SetIsPaused(paused);
		}
	}
}

void PauseManager::PauseRocks(bool paused)
{
	for (int i = 0; i < rockGameObjects.size(); ++i)
	{
		rockGameObjects[i]->GetComponent<BossChargeRockScript>()->SetPauseRock(paused);
	}
}

void PauseManager::PauseMissiles(bool paused)
{
	for (int i = 0; i < missileGameObjects.size(); ++i)
	{
		missileGameObjects[i]->GetComponent<BossMissilesMissileScript>()->SetIsPaused(paused);
	}
}
