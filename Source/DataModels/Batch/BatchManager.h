#pragma once

#include <vector>

class GeometryBatch;
class ComponentMeshRenderer;

class BatchManager
{
public:
	BatchManager();
	~BatchManager();

	void AddComponent(ComponentMeshRenderer* newComponent);
private:
	GeometryBatch* CheckBatchCompatibility(const ComponentMeshRenderer* newComponent);

	std::vector<GeometryBatch*> geometryBatches;
};