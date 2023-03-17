#pragma once

#include "FileSystem/UniqueID.h"
#include "Math/float2.h"
#include "Math/float3.h"

#include <vector>
#include <list>

#include "GL/glew.h"

class ComponentMeshRenderer;
class ResourceMesh;
class GameObject;

struct Command
{
	GLuint  count;			// Number of indices in the mesh
	GLuint  instanceCount;	// Number of instances to render
	GLuint  firstIndex;		// Index offset in the EBO
	GLuint  baseVertex;		// Vertex offset in the VBO
	GLuint  baseInstance;	// Instance Index
};

struct ResourceInfo //temporary name
{
	ResourceMesh* resourceMesh;
	int vertexOffset;
	int indexOffset;
	int timesRepeated;
};

class GeometryBatch
{
public:
	GeometryBatch();
	~GeometryBatch();

	void CreateVAO();

	void AddComponentMeshRenderer(ComponentMeshRenderer* newComponent);
	void RemoveComponent(ComponentMeshRenderer* component); //maybe delete

	void BindBatch(std::vector<ComponentMeshRenderer*>& componentsToRender);

	const int GetFlags() const;
	const int GetResourceIndex() const;

	bool CleanUp();

private:
	void FillBuffers();
	void FillEBO();

	const GameObject* GetComponentOwner(const ResourceMesh* resourceMesh); //delete

	void CreateOrCountInstance(ResourceMesh* resourceMesh);
	void RecalculateInfoResource(); //maybe delete

	ResourceInfo& FindResourceMesh(ResourceMesh* mesh);

	std::list<ComponentMeshRenderer*> components; //maybe return to vector
	std::list<ResourceInfo> resourcesInfo; //maybe return to vector

	unsigned int ebo = 0;
	unsigned int vao = 0;
	
	unsigned int indirectBuffer = 0;
	unsigned int verticesBuffer = 0;
	unsigned int textureBuffer = 0;
	unsigned int normalsBuffer = 0;
	unsigned int tangentsBuffer = 0;
	unsigned int transforms = 0;

	std::vector<float3> verticesToRender;
	std::vector<float2> texturesToRender;
	std::vector<float3> normalsToRender;
	std::vector<float3> tangentsToRender;

	unsigned int resourceMeshIndex = 0;

	std::vector<Command> commands;

	int numTotalVertices = 0;
	int numTotalIndices = 0;
	int numTotalFaces = 0;

	int flags = 0;
};

inline const int GeometryBatch::GetFlags() const
{
	return flags;
}

inline const int GeometryBatch::GetResourceIndex() const
{
	return resourceMeshIndex;
}
