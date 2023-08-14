#pragma once

#include "Scripting\Script.h"

// This script performs a seeking behaviour once the player enters in range

class ComponentTransform;
class AIMovement;

class SeekBehaviourScript : public Script
{
public:
	SeekBehaviourScript();
	~SeekBehaviourScript() override = default;

	void Start() override;

	void Seeking() const;
	void DisableMovement() const;
	void DisableRotation() const;
	void RotateToTarget() const;

	GameObject* GetTarget() const;

private:
	GameObject* target;

	ComponentTransform* targetTransform;
	ComponentTransform* ownerTransform;
	AIMovement* aiMovement;
};
