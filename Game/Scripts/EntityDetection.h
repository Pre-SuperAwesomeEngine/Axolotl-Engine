#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

class ModuleInput;
class ComponentRigidBody;
class ComponentTransform;
class GameObject;

class EntityDetection : public Script
{

public:
	EntityDetection();
	~EntityDetection() override = default;

	void Start() override;

	void UpdateEnemyDetection(float distanceFilter = 0.0f);
	void FilterEnemiesByDistance(float distanceFilter, std::vector<ComponentRigidBody*>& enemiesInTheAreaFiltered);

	virtual void OnCollisionEnter(ComponentRigidBody* other) override;
	virtual void OnCollisionExit(ComponentRigidBody* other) override;

	GameObject* GetEnemySelected() const;
	bool AreAnyEnemiesInTheArea() const;

	std::vector<ComponentRigidBody*>& GetEnemiesInTheArea();

private:
	void DrawDetectionLines(float distanceFilter);
	void SelectEnemy(float distanceFilter);

	void VisualParticle(bool activate, GameObject* enemy);

	ModuleInput* input;

	GameObject* player;
	ComponentTransform* playerTransform;
	ComponentRigidBody* rigidBody;

	float interactionAngle;
	float interactionOffset;
	float angleThresholdEnemyIntersection;

	std::vector<ComponentRigidBody*> enemiesInTheArea;
	ComponentTransform* enemySelected;

	float3 vecForward;
	float3 originPosition;
};