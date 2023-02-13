#pragma once
#include <list>
#include <map>
#include <memory>
#include <MathGeoLib/Include/Geometry/AABB.h>

#include "Globals.h"
#include "Geometry/LineSegment.h"

class GameObject;

class Quadtree
{
public:
	Quadtree(const AABB& boundingBox);
	Quadtree(const AABB& boundingBox, Quadtree* root);
	~Quadtree();

	bool IsLeaf() const;
	bool InQuadrant(const std::shared_ptr<GameObject>& gameObject);

	void Add(const std::shared_ptr<GameObject>& gameObject);
	void AddGameObjectAndChildren(const std::shared_ptr<GameObject>& gameObject);
	void Remove(const std::weak_ptr<GameObject>& gameObject);
	void RemoveGameObjectAndChildren(const std::weak_ptr<GameObject>& gameObject);
	void SmartRemove();
	void OptimizeParentObjects();

	void Subdivide();
	void RedistributeGameObjects(const std::shared_ptr<GameObject>& gameobject);

	void ExpandToFit(const std::shared_ptr<GameObject>& gameObject);
	void AdjustHeightToNodes(float minY, float maxY);

	void ResetChildren();

	const std::list<std::weak_ptr<GameObject> >& GetGameObjects() const;
	void GetFamilyObjects(std::list<std::weak_ptr<GameObject> >& familyGameObjects);

	const Quadtree* GetFrontRightNode() const;
	const Quadtree* GetFrontLeftNode() const;
	const Quadtree* GetBackRightNode() const;
	const Quadtree* GetBackLeftNode() const;

	bool IsFreezed() const;
	void SetFreezedStatus(bool isFreezed);

	int GetQuadrantCapacity() const;
	void SetQuadrantCapacity(int quadrantCapacity);

	float GetMinQuadrantSideSize() const;
	void SetMinQuadrantSideSize(float minCubeSize);

	const AABB& GetBoundingBox() const;
	void SetBoundingBox(AABB boundingBox);

	std::list<std::weak_ptr<GameObject> > GetAllGameObjects(const std::weak_ptr<GameObject>& gameObject);

	// Speeding raycast function, this should be changed to an iterative function instead of a recursive function
	void CheckRaycastIntersection(std::map<float, std::weak_ptr<GameObject>>& hittedGameObjects,
									const LineSegment& ray);

private:

	std::list<std::weak_ptr<GameObject> > gameObjects;
	AABB boundingBox;

	int quadrantCapacity = QUADRANT_CAPACITY;
	float minQuadrantSideSize = MIN_CUBE_SIZE;
	float minQuadrantDiagonalSquared = 3 * MIN_CUBE_SIZE * MIN_CUBE_SIZE; // D^2 = 3C^2

	Quadtree* parent;

	std::unique_ptr<Quadtree> frontRightNode = nullptr;
	std::unique_ptr<Quadtree> frontLeftNode = nullptr;
	std::unique_ptr<Quadtree> backRightNode = nullptr;
	std::unique_ptr<Quadtree> backLeftNode = nullptr;

	bool isFreezed = false;
};

inline bool Quadtree::IsFreezed() const
{
	return isFreezed;
}

inline void Quadtree::SetFreezedStatus(bool isFreezed)
{
	this->isFreezed = isFreezed;
}

inline const AABB& Quadtree::GetBoundingBox() const
{
	return boundingBox;
}

inline void Quadtree::SetBoundingBox(AABB boundingBox)
{
	this->boundingBox = boundingBox;
}

inline int Quadtree::GetQuadrantCapacity() const
{
	return quadrantCapacity;
}

inline float Quadtree::GetMinQuadrantSideSize() const
{
	return minQuadrantSideSize;
}

inline const std::list<std::weak_ptr<GameObject> >& Quadtree::GetGameObjects() const
{
	return gameObjects;
}

inline const Quadtree* Quadtree::GetFrontRightNode() const
{
	return frontRightNode.get();
}

inline const Quadtree* Quadtree::GetFrontLeftNode() const
{
	return frontLeftNode.get();
}

inline const Quadtree* Quadtree::GetBackRightNode() const
{
	return backRightNode.get();
}

inline const Quadtree* Quadtree::GetBackLeftNode() const
{
	return backLeftNode.get();
}

inline void Quadtree::SetQuadrantCapacity(int quadrantCapacity)
{
	this->quadrantCapacity = quadrantCapacity;
}

inline void Quadtree::SetMinQuadrantSideSize(float minQuadrantSideSize)
{
	this->minQuadrantSideSize = minQuadrantSideSize;
	minQuadrantDiagonalSquared = 3 * minQuadrantSideSize * minQuadrantSideSize;
}

