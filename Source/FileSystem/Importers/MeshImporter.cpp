#pragma warning (disable: 6386)

#include "MeshImporter.h"

#include "Application.h"
#include "FileSystem/ModuleFileSystem.h"
#include "Resources/ResourceMesh.h"

MeshImporter::MeshImporter()
{
}

MeshImporter::~MeshImporter()
{
}

void MeshImporter::Import(const char* filePath, std::shared_ptr<ResourceMesh> resource)
{
	char* loadBuffer{};
	App->fileSystem->Load(filePath, loadBuffer);
	Load(loadBuffer, resource);

	char* saveBuffer{};
	unsigned int size;
	Save(resource, saveBuffer, size);
	App->fileSystem->Save((resource->GetLibraryPath() + GENERAL_BINARY_EXTENSION).c_str(), saveBuffer, size);

	delete loadBuffer;
	delete saveBuffer;
}

void MeshImporter::Save(const std::shared_ptr<ResourceMesh>& resource, char* &fileBuffer, unsigned int& size)
{
	unsigned int hasTangents = !resource->GetTangents().empty();
	unsigned int header[5] =
	{
		resource->GetNumFaces(),
		resource->GetNumVertices(),
		resource->GetNumBones(),
		resource->GetMaterialIndex(),
		hasTangents
	};
	
	unsigned int sizeOfVectors = sizeof(float3) * resource->GetNumVertices();
	unsigned int numOfVectors = 3;
	if (hasTangents)
	{
		numOfVectors = 4;
	}
	size = sizeof(header) + resource->GetNumFaces() * (sizeof(unsigned int) * 3)
		+ static_cast<unsigned long long>(sizeOfVectors) * static_cast<unsigned long long>(numOfVectors)
		+ resource->GetNumBones() * sizeof(unsigned int) + resource->GetNumBones() * sizeof(float4x4)
		+ resource->GetNumVertices() * (4 * sizeof(unsigned int) + 4 * sizeof(float));

	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		size += (*resource->GetBones()[i]).name.size();
	}
	
	char* cursor = new char[size];
	
	fileBuffer = cursor;
	
	unsigned int bytes = sizeof(header);
	memcpy(cursor, header, bytes);

	cursor += bytes;

	if (!resource->GetVertices().empty())
	{
		bytes = sizeof(float3) * resource->GetNumVertices();
		memcpy(cursor, &(resource->GetVertices()[0]), bytes);

		cursor += bytes;
	}

	if (!resource->GetTextureCoords().empty())
	{
		bytes = sizeof(float3) * resource->GetNumVertices();
		memcpy(cursor, &(resource->GetTextureCoords()[0]), bytes);

		cursor += bytes;
	}

	if (!resource->GetNormals().empty())
	{
		bytes = sizeof(float3) * resource->GetNumVertices();
		memcpy(cursor, &(resource->GetNormals()[0]), bytes);

		cursor += bytes;
	}

	if (!resource->GetTangents().empty())
	{
		bytes = sizeof(float3) * resource->GetNumVertices();
		memcpy(cursor, &(resource->GetTangents()[0]), bytes);

		cursor += bytes;
	}

	for (unsigned int i = 0; i < resource->GetNumFaces(); ++i)
	{
		bytes = sizeof(unsigned int) * 3;
		memcpy(cursor, &(resource->GetFacesIndices()[i][0]), bytes);

		cursor += bytes;
	}

	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		bytes = sizeof(float4x4);
		memcpy(cursor, &((*resource->GetBones()[i]).transform), bytes);

		cursor += bytes;

		bytes = (*resource->GetBones()[i]).name.size();
		memcpy(cursor, &((*resource->GetBones()[i]).name), bytes);
		
		cursor += bytes;
	}

	for (unsigned int i = 0; i < resource->GetNumVertices(); ++i)
	{
		bytes = 4 * sizeof(unsigned int);
		memcpy(cursor, &(*resource->GetAttaches()[i]->bones), bytes);
		
		cursor += bytes;

		bytes = 4 * sizeof(float);
		memcpy(cursor, &(*resource->GetAttaches()[i]->weights), bytes);

		cursor += bytes;
	}
}

void MeshImporter::Load(const char* fileBuffer, std::shared_ptr<ResourceMesh> resource)
{
	unsigned int header[5];
	memcpy(header, fileBuffer, sizeof(header));

	resource->SetNumFaces(header[0]);
	resource->SetNumVertices(header[1]);
	resource->SetNumBones(header[2]);
	resource->SetMaterialIndex(header[3]);
	bool hasTangents = header[4];

	fileBuffer += sizeof(header);

	float3* vertexPointer = new float3[resource->GetNumVertices()];
	unsigned int bytes = sizeof(float3) * resource->GetNumVertices();
	memcpy(vertexPointer, fileBuffer, bytes);
	std::vector<float3> vertexs(vertexPointer, vertexPointer + resource->GetNumVertices());
	resource->SetVertices(vertexs);

	fileBuffer += bytes;

	delete[] vertexPointer;

	float3* textureCoordPointer = new float3[resource->GetNumVertices()];
	bytes = sizeof(float3) * resource->GetNumVertices();
	memcpy(textureCoordPointer, fileBuffer, bytes);
	std::vector<float3> textureCoord(textureCoordPointer, textureCoordPointer + resource->GetNumVertices());
	resource->SetTextureCoords(textureCoord);

	fileBuffer += bytes;

	delete[] textureCoordPointer;

	float3* normalPointer = new float3[resource->GetNumVertices()];
	bytes = sizeof(float3) * resource->GetNumVertices();
	memcpy(normalPointer, fileBuffer, bytes);
	std::vector<float3> normals(normalPointer, normalPointer + resource->GetNumVertices());
	resource->SetNormals(normals);

	fileBuffer += bytes;

	delete[] normalPointer;

	if(hasTangents)
	{
		float3* tangentPointer = new float3[resource->GetNumVertices()];
		bytes = sizeof(float3) * resource->GetNumVertices();
		memcpy(tangentPointer, fileBuffer, bytes);
		std::vector<float3> tangents(tangentPointer, tangentPointer + resource->GetNumVertices());
		resource->SetTangents(tangents);

		fileBuffer += bytes;

		delete[] tangentPointer;
	}

	int size = 3 * resource->GetNumFaces();
	unsigned int* indexesPointer = new unsigned int[size];
	bytes = resource->GetNumFaces() * sizeof(unsigned int) * 3;
	memcpy(indexesPointer, fileBuffer, bytes);
	std::vector<unsigned int> aux(indexesPointer, indexesPointer + resource->GetNumFaces() * 3);
	std::vector<std::vector<unsigned int>> faces;
	for (unsigned int i = 0; i + 2 < (resource->GetNumFaces() * 3); i += 3) 
	{
		std::vector<unsigned int> indexes{ aux[i], aux[i + 1], aux[i + 2] };
		faces.push_back(indexes);
	}
	resource->SetFacesIndices(faces);

	fileBuffer += bytes;

	delete[] indexesPointer;

	float4x4* transformPointer = new float4x4[resource->GetNumBones()];
	bytes = resource->GetNumBones() * sizeof(float4x4);
	memcpy(transformPointer, fileBuffer, bytes);
	fileBuffer += bytes;

	std::vector<unsigned int> nameSizes;
	unsigned int nameSize = 0u;
	unsigned int auxNameSize = 0u;
	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		while ((fileBuffer[i + nameSize + auxNameSize]) != '\0')
		{
			++auxNameSize;
		}

		nameSizes.push_back(auxNameSize);
		nameSize += auxNameSize;
		auxNameSize = 0;
	}

	char* namePointer = new char[nameSize];
	bytes = nameSize;
	memcpy(namePointer, fileBuffer, bytes);
	fileBuffer += bytes;

	std::string auxName;
	float4x4 auxTransform;
	std::vector<Bone*> bones;
	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		memcpy(&auxName, namePointer, nameSizes.at(i));

		namePointer += nameSizes.at(i);

		memcpy(&auxTransform, transformPointer, sizeof(float4x4));
		
		transformPointer += sizeof(float4x4);

		bones.push_back(new Bone(auxTransform, auxName));
	}
	resource->SetBones(bones);

	delete[] namePointer;
	delete[] transformPointer;

	size = 4 * resource->GetNumVertices();
	unsigned int* bonePointer = new unsigned int[size];
	bytes = 4 * resource->GetNumVertices() * sizeof(unsigned int);
	memcpy(bonePointer, fileBuffer, bytes);
	fileBuffer += bytes;

	float* weightPointer = new float[size];
	bytes = 4 * resource->GetNumVertices() * sizeof(float);
	memcpy(weightPointer, fileBuffer, bytes);
	fileBuffer += bytes;

	unsigned int auxBone[4] = {0u, 0u, 0u, 0u};
	float auxWeight[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	std::vector<Attach*> attaches;
	for (unsigned int i = 0; i + 3 < (resource->GetNumVertices() * 4); i += 4)
	{
		bytes = 4 * sizeof(unsigned int);
		memcpy(&auxBone, bonePointer + i, bytes);

		bytes = 4 * sizeof(float);
		memcpy(&auxWeight, weightPointer + i, bytes);

		attaches.push_back(new Attach(auxBone, auxWeight));
	}
	resource->SetAttaches(attaches);

	delete[] bonePointer;
	delete[] weightPointer;
}