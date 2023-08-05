#pragma once
#include "Script.h"

class ComponentTransform;
class ComponentAudioSource;
class ComponentRigidBody;

class LightAttackBullet : public Script
{
public:
	LightAttackBullet();
	~LightAttackBullet() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void SetBulletVelocity(float nVelocity);
	void SetStunTime(float nStunTime);
	void SetEnemy(GameObject* nEnemy);

private:
	void OnCollisionEnter(ComponentRigidBody* other) override;
	void DestroyBullet();

	float velocity;
	float stunTime;
	float damageAttack;
	float maxDistanceBullet;

	float3 defaultTargetPos;

	GameObject* enemy;
	ComponentAudioSource* audioSource;
	ComponentRigidBody* rigidBody;
	ComponentTransform* parentTransform;
};
