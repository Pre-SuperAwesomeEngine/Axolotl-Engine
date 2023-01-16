#pragma once
#include "FileSystem/Importers/Importer.h"
#include "DataModels/Resources/ResourceMesh.h"

class MeshImporter : public Importer<ResourceMesh>
{
public:
    MeshImporter() = default;
    ~MeshImporter() = default;

    void Import(const char* filePath, std::shared_ptr<ResourceMesh> resource) override;
    void Load(const char* fileBuffer, std::shared_ptr<ResourceMesh> resource) override;

protected:
    uint64_t Save(const std::shared_ptr<ResourceMesh>& resource, char* &fileBuffer, unsigned int& size) override;
};
