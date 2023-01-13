#include "MaterialImporter.h"
#include "Application.h"
#include "FileSystem/ModuleFileSystem.h"
#include "FileSystem/ModuleResources.h"


void MaterialImporter::Import(const char* filePath, std::shared_ptr<ResourceMaterial> resource)
{
	char* buffer;

	App->fileSystem->Load(filePath, buffer);

	unsigned int header[4];
	memcpy(header, buffer, sizeof(header));

	buffer += sizeof(header);

	std::vector<UID> resourceTexture;

	for (int i = 0; i < 4; ++i)
	{
		char* pathPointer = new char[header[i]];
		memcpy(pathPointer, buffer, header[i]);
		std::string path(pathPointer, pathPointer + header[i]);

		if (!path.empty()) 
		{
			resourceTexture.push_back(App->resources->ImportResource(path));
		}
		else 
		{
			resourceTexture.push_back(0);
		}

		buffer += header[i];
	}

	if(resourceTexture[0] != 0) resource->SetDiffuseUID(resourceTexture[0]);
	if(resourceTexture[1] != 0) resource->SetNormalUID(resourceTexture[1]);
	if(resourceTexture[2] != 0) resource->SetOcclusionUID(resourceTexture[2]);
	if(resourceTexture[3] != 0) resource->SetSpecularUID(resourceTexture[3]);

}

uint64_t MaterialImporter::Save(const std::shared_ptr<ResourceMaterial>& resource, char*& fileBuffer, unsigned int& size)
{
    UID texturesUIDs[4] = 
	{ 
		resource->GetDiffuseUID(),
		resource->GetNormalUID(),
		resource->GetOcclusionrUID(),
		resource->GetSpecularUID()
	};

	float3 colors[2] =
	{
		resource->GetDiffuseColor(),
		resource->GetSpecularColor()
	};

	size = sizeof(texturesUIDs) + sizeof(colors) + sizeof(float);

	char* cursor = new char[size];

	fileBuffer = cursor;

	unsigned int bytes = sizeof(texturesUIDs);
	memcpy(cursor, texturesUIDs, bytes);

	cursor += bytes;

	bytes = sizeof(colors);
	memcpy(cursor, colors, bytes);

	cursor += bytes;

	bytes = sizeof(float);
	memcpy(cursor, &resource->GetShininess(), bytes);

	// Provisional return, here we have to return serialize UID for the object
	return 0;
}

void MaterialImporter::Load(const char* fileBuffer, std::shared_ptr<ResourceMaterial> resource)
{
	UID texturesUIDs[4];
	memcpy(texturesUIDs, fileBuffer, sizeof(texturesUIDs));

	resource->SetDiffuseUID(texturesUIDs[0]);
	resource->SetNormalUID(texturesUIDs[1]);
	resource->SetOcclusionUID(texturesUIDs[2]);
	resource->SetSpecularUID(texturesUIDs[3]);

	fileBuffer += sizeof(texturesUIDs);

	float3 colors[2];
	memcpy(colors, fileBuffer, sizeof(colors));

	resource->SetDiffuseColor(colors[0]);
	resource->SetSpecularColor(colors[1]);

	fileBuffer += sizeof(colors);

	float* shininess = new float;
	memcpy(shininess, fileBuffer, sizeof(float));
	resource->SetShininess(*shininess);
}
