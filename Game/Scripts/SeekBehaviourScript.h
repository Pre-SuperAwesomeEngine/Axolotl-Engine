#pragma once

#include "Scripting\Script.h"

// This script performs a seeking behaviour once the player enters in range

class ComponentTransform;
class ComponentRigidBody;

class SeekBehaviourScript : public Script
{
public:
	SeekBehaviourScript();
	~SeekBehaviourScript() override = default;

	void Start() override;

	void Seeking() const;

private:
	GameObject* target;

	ComponentTransform* targetTransform;
	ComponentRigidBody* ownerRigidBody;
};