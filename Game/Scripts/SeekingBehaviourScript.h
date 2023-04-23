#pragma once

#include "Scripting\Script.h"

// This script performs a seeking behaviour once the player enters in range

class ComponentTransform;
class ComponentRigidbody;

class SeekingBehaviourScript : public Script
{
public:
	SeekingBehaviourScript();
	~SeekingBehaviourScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	GameObject* GetTarget() const;
	void SetTarget(GameObject* target);

private:
	GameObject* target;

	//ComponentTransform* targetTransform;
	//ComponentRigidBody* ownerRigidBody;
};