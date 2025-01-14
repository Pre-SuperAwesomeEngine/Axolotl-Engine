#include "StdAfx.h"

#include "MeshImporter.h"

#include "Application.h"
#include "FileSystem/ModuleFileSystem.h"
#include "Resources/ResourceMesh.h"

#include "Defines/ExtensionDefines.h"

MeshImporter::MeshImporter()
{
}

MeshImporter::~MeshImporter()
{
}

void MeshImporter::Import(const char* filePath, std::shared_ptr<ResourceMesh> resource)
{
	char* loadBuffer{};
	ModuleFileSystem* fileSystem = App->GetModule<ModuleFileSystem>();
	fileSystem->Load(filePath, loadBuffer);
	Load(loadBuffer, resource);

	char* saveBuffer{};
	unsigned int size;
	Save(resource, saveBuffer, size);
	fileSystem->Save((resource->GetLibraryPath() + GENERAL_BINARY_EXTENSION).c_str(), saveBuffer, size);

	delete loadBuffer;
	delete saveBuffer;
}

void MeshImporter::Save(const std::shared_ptr<ResourceMesh>& resource, char*& fileBuffer, unsigned int& size)
{
	unsigned int hasTangents = !resource->GetTangents().empty();
	unsigned int header[5] = { resource->GetNumFaces(),
							   resource->GetNumVertices(),
							   resource->GetNumBones(),
							   resource->GetMaterialIndex(),
							   hasTangents };

	unsigned int sizeOfVectors = sizeof(float3) * resource->GetNumVertices();
	unsigned int numOfVectors = 3;
	if (hasTangents)
	{
		numOfVectors = 4;
	}
	size = sizeof(header) + resource->GetNumFaces() * (sizeof(unsigned int) * 3) +
		   static_cast<unsigned long long>(sizeOfVectors) * static_cast<unsigned long long>(numOfVectors) +
		   resource->GetNumBones() * (sizeof(float4x4) + sizeof(unsigned int) * 2);

	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		size += static_cast<unsigned int>(resource->GetBones()[i].name.size());
		size += resource->GetNumWeights()[i] * (sizeof(unsigned int) + sizeof(float));
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
		memcpy(cursor, &(resource->GetBones()[i].transform), bytes);

		cursor += bytes;

		bytes = sizeof(unsigned int);
		unsigned int sizeNameHeader = static_cast<unsigned int>(resource->GetBones()[i].name.size());
		memcpy(cursor, &sizeNameHeader, bytes);

		cursor += bytes;

		bytes = sizeNameHeader;
		memcpy(cursor, resource->GetBones()[i].name.c_str(), bytes);

		cursor += bytes;

		bytes = sizeof(unsigned int);
		memcpy(cursor, &(resource->GetNumWeights()[i]), bytes);

		cursor += bytes;

		for (unsigned int j = 0; j < resource->GetNumVertices(); ++j)
		{
			for (unsigned int k = 0; k < resource->GetAttaches()[j].numBones; ++k)
			{
				if (resource->GetAttaches()[j].bones[k] == i)
				{
					bytes = sizeof(unsigned int);
					memcpy(cursor, &(j), bytes);

					cursor += bytes;

					bytes = sizeof(float);
					memcpy(cursor, &(resource->GetAttaches()[j].weights[k]), bytes);

					cursor += bytes;
				}
			}
		}
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

	if (hasTangents)
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

	std::vector<Bone> bones;
	std::vector<unsigned int> allNumWeights;

	resource->SetAttachResize();

	unsigned int vertexId = 0u;
	float vertexWeight = 0.0f;
	unsigned int numWeights = 0u;

	unsigned int bonesPerVertex = resource->GetBonesPerVertex();

	for (unsigned int i = 0; i < resource->GetNumBones(); ++i)
	{
		Bone bone;

		memcpy(&bone.transform, fileBuffer, sizeof(float4x4));
		fileBuffer += sizeof(float4x4);

		unsigned int sizeOfName;
		bytes = sizeof(unsigned int);
		memcpy(&sizeOfName, fileBuffer, bytes);
		fileBuffer += bytes;

		char* name = new char[sizeOfName]{};
		bytes = sizeOfName;
		memcpy(name, fileBuffer, bytes);
		fileBuffer += bytes;
		bone.name = std::string(name, sizeOfName);
		delete[] name;

		bones.push_back(bone);

		memcpy(&numWeights, fileBuffer, sizeof(unsigned int));
		fileBuffer += sizeof(unsigned int);

		unsigned int numWeightsAfterBonesLimit = 0;
		for (unsigned int j = 0; j < numWeights; ++j)
		{
			memcpy(&vertexId, fileBuffer, sizeof(unsigned int));
			if (resource->GetNumBonesAttached(vertexId) < bonesPerVertex)
			{
				resource->SetAttachBones(vertexId, i);
				numWeightsAfterBonesLimit++;
			}
			fileBuffer += sizeof(unsigned int);

			memcpy(&vertexWeight, fileBuffer, sizeof(float));
			if (resource->GetNumBonesAttached(vertexId) < bonesPerVertex)
			{
				resource->SetAttachWeight(vertexId, vertexWeight);
			}
			fileBuffer += sizeof(float);

			resource->IncrementAttachNumBones(vertexId);
		}
		allNumWeights.push_back(numWeightsAfterBonesLimit);
	}

	for (unsigned int i = 0; i < resource->GetAttaches().size(); ++i)
	{
		resource->NormalizeWeights(i);
	}

	resource->SetBones(bones);
	resource->SetNumWeights(allNumWeights);
}