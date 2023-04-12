#pragma once

#include "Resource.h"

#include "Components/ComponentMeshRenderer.h"

#include "Math/float4x4.h"

#include <vector>
#include <memory>

class ResourceMesh;
class ResourceMaterial;
class ResourceAnimation;

struct OptionsModel
{
};

class ResourceModel : virtual public Resource
{
public:
	struct Node
	{
		std::string name;
		float4x4 transform;
		int parent;
		std::vector<std::pair<std::shared_ptr<ResourceMesh>, std::shared_ptr<ResourceMaterial>>> meshRenderers;
	};

public:
	ResourceModel(UID resourceUID, 
		const std::string& fileName, 
		const std::string& assetsPath, 
		const std::string& libraryPath);
	virtual ~ResourceModel() override;

	ResourceType GetType() const override;

	void SaveImporterOptions(Json& meta) override {};
	void SaveLoadOptions(Json& meta) override {};

	void LoadImporterOptions(Json& meta) override {};
	void LoadLoadOptions(Json& meta) override {};

	const size_t GetNumMeshes() const;
	const size_t GetNumMaterials() const;
	const size_t GetNumNodes() const;
	const size_t GetNumAnimations() const;
	const std::vector<Node*>& GetNodes() const;
	const std::vector<std::shared_ptr<ResourceMesh>>& GetMeshes() const;
	const std::vector<std::shared_ptr<ResourceMaterial>>& GetMaterials() const;
	const std::vector<std::shared_ptr<ResourceAnimation>>& GetAnimations() const;
	OptionsModel& GetOptions();

	void SetNodes(const std::vector<Node*>& nodes);
	void SetAnimations(const std::vector<std::shared_ptr<ResourceAnimation>>& animations);

	void AppendNode(Node* node);


protected:
	void InternalLoad() override;
	void InternalUnload() override;

private:
	OptionsModel options;

	std::vector<Node*> nodes;
	std::vector<std::shared_ptr<ResourceAnimation>> animations;
};

inline ResourceType ResourceModel::GetType() const
{
	return ResourceType::Model;
}

inline const size_t ResourceModel::GetNumNodes() const
{
	return nodes.size();
}

inline const std::vector<ResourceModel::Node*>& ResourceModel::GetNodes() const
{
	return nodes;
}

inline OptionsModel& ResourceModel::GetOptions()
{
	return options;
}

inline void ResourceModel::SetNodes(const std::vector<Node*>& nodes)
{
	this->nodes = nodes;
}

inline const std::vector<std::shared_ptr<ResourceAnimation>>& ResourceModel::GetAnimations() const
{
	return animations;
}

inline const size_t ResourceModel::GetNumAnimations() const
{
	return animations.size();
}

inline void ResourceModel::SetAnimations(const std::vector<std::shared_ptr<ResourceAnimation>>& animations)
{
	this->animations = animations;
}


inline void ResourceModel::AppendNode(Node* node)
{
	nodes.push_back(node);
}