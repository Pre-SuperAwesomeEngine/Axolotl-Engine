#include "GeometryBatch.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleCamera.h"

#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentTransform.h"
#include "GameObject/GameObject.h"
#include "Resources/ResourceMesh.h"
#include "DataModels/Batch/BatchFlags.h"

#include <gl/glew.h>
#include <assert.h>

GeometryBatch::GeometryBatch()
{
	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}
	//TODO complete
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
	CreateVAO();
}

GeometryBatch::~GeometryBatch()
{
	components.clear();
	resourceMeshes.clear();
	CleanUp();
}

void GeometryBatch::CalculateVBO()
{
	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	GLuint bufferSize = CalculateSpaceInVBO() * numTotalVertices;

	glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

	int spaceUsed = 0;

	for (auto aaa : resourceMeshes)
	{
		ResourceMesh* resourceMesh = aaa.resourceMesh;

		GLuint positionSize = sizeof(float) * 3 * resourceMesh->GetNumVertices();

		glBufferSubData(GL_ARRAY_BUFFER, spaceUsed, positionSize, &(resourceMesh->GetVertices()[0]));

		GLuint uvOffset = positionSize + spaceUsed;
		GLuint uvSize = sizeof(float) * 2 * resourceMesh->GetNumVertices();

		float2* uvs = (float2*)(glMapBufferRange(GL_ARRAY_BUFFER, uvOffset, uvSize, GL_MAP_WRITE_BIT));

		for (unsigned int i = 0; i < resourceMesh->GetNumVertices(); ++i)
		{
			uvs[i] = float2(resourceMesh->GetTextureCoords()[i].x, resourceMesh->GetTextureCoords()[i].y);
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);

		unsigned normalsOffset = positionSize + uvSize + spaceUsed;
		unsigned normalsSize = sizeof(float) * 3 * resourceMesh->GetNumVertices();
		glBufferSubData(GL_ARRAY_BUFFER, normalsOffset, normalsSize, &resourceMesh->GetNormals()[0]);

		if (resourceMesh->GetTangents().size() != 0)
		{
			unsigned tangentsOffset = positionSize + uvSize + normalsSize + spaceUsed;
			unsigned tangentsSize = sizeof(float) * 3 * resourceMesh->GetNumVertices();
			glBufferSubData(GL_ARRAY_BUFFER, tangentsOffset, tangentsSize, &resourceMesh->GetTangents()[0]);
			spaceUsed += tangentsSize;
		}
		spaceUsed += positionSize + uvSize + normalsSize;
	}
}

void GeometryBatch::CalculateEBO()
{
	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	GLuint indexSize = sizeof(GLuint) * numTotalFaces * 3;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, nullptr, GL_STATIC_DRAW);

	GLuint* indices = (GLuint*)(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

	for (auto aaa : resourceMeshes)
	{
		for (unsigned int i = 0; i < aaa.resourceMesh->GetNumFaces(); ++i)
		{
			assert(aaa.resourceMesh->GetFacesIndices()[i].size() == 3); // note: assume triangles = 3 indices per face
			*(indices++) = aaa.resourceMesh->GetFacesIndices()[i][0];
			*(indices++) = aaa.resourceMesh->GetFacesIndices()[i][1];
			*(indices++) = aaa.resourceMesh->GetFacesIndices()[i][2];
		}	
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void GeometryBatch::CreateVAO()
{
	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}
	glBindVertexArray(vao);
	//verify which data to send in buffer

	//vertices
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3, static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);
	
	//texture
	glGenBuffers(1, &textureBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(1);
	
	//normals
	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(2);
	
	if (flags & HAS_TANGENTS)
	{
		glGenBuffers(1, &tangentsBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, tangentsBuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
		glEnableVertexAttribArray(3);
	}
	//tangents
	

	glGenBuffers(1, &indirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);

	glBindVertexArray(0);
}

void GeometryBatch::UpdateVAO()
{
	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}

	glBindVertexArray(vao);

	//vertices
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, verticesToRender.size() * sizeof(float), &verticesToRender[0], GL_DYNAMIC_DRAW);

	//texture
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, texturesToRender.size() * sizeof(float), &texturesToRender[0], GL_DYNAMIC_DRAW);

	//normals
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normalsToRender.size() * sizeof(float), &normalsToRender[0], GL_DYNAMIC_DRAW);

	//tangents
	if (flags & HAS_TANGENTS)
	{
		glBindBuffer(GL_ARRAY_BUFFER, tangentsBuffer);
		glBufferData(GL_ARRAY_BUFFER, tangentsToRender.size() * sizeof(float), &tangentsToRender[0], GL_DYNAMIC_DRAW);
	}
	
	//indirect
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), &commands[0], GL_DYNAMIC_DRAW);
}

void GeometryBatch::AddComponentMeshRenderer(ComponentMeshRenderer* newComponent)
{
	if (newComponent)
	{
		std::shared_ptr<ResourceMesh> meshShared = newComponent->GetMesh();
		if (!meshShared)
		{
			return;
		}

		ResourceMesh* mesh = meshShared.get();
		if (components.empty())
		{
			if (mesh->GetNormals().size() != 0)
			{
				flags |= HAS_NORMALS;
			}

			if (mesh->GetTextureCoords().size() != 0)
			{
				flags |= HAS_TEXTURE_COORDINATES;
			}

			if (mesh->GetTangents().size() != 0)
			{
				flags |= HAS_TANGENTS;
			}
		}

		if (IsUniqueResourceMesh(mesh))
		{
			AAA aaa = {
				mesh,
				numTotalVertices,
				numTotalFaces
			};
			resourceMeshes.push_back(aaa);
			numTotalVertices += mesh->GetNumVertices();
			numTotalFaces += mesh->GetNumFaces();
			CalculateVBO();
			CalculateEBO();
		}
		newComponent->SetBatch(this);
		components.push_back(newComponent);
	}
}

void GeometryBatch::BindBatch()
{
	// Set up the vertex data
	float vertices[] = {
		// First triangle
		-0.5f, -0.5f, 0.0f,  // Botton left
		-0.5f, 0.5f, 0.0f,  // Top left
		 0.5f, -0.5f, 0.0f,  // Bottom right
		 0.5f, 0.5f, 0.0f  // top right
	};

	// Set up the index data
	GLuint indices[] = {
		0, 2, 3, // First triangle
		0, 3, 1, // Second triangle
	};

	// Create the vertex buffer and load the vertex data into it
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	// Create the index buffer and load the index data into it
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

	// Set up the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	// Create the draw commands buffer
	Command drawCommands[] = {
		{3, 1, 0, 0, 0}, // First triangle
		{3, 1, 3, 0, 0}, // Second triangle
	};

	// Create the draw commands buffer and load the draw commands into it
	glGenBuffers(1, &indirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(drawCommands), drawCommands, GL_DYNAMIC_DRAW);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)0, 2, 0);
}

void GeometryBatch::BindBatch2(std::vector<ComponentMeshRenderer*>& componentsToRender)
{

	unsigned program = App->program->GetProgram();
	GLint programInUse;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

	if (program != programInUse)
	{
		glUseProgram(program);
	}

	commands.clear();
	commands.reserve(components.size());

	verticesToRender.clear();
	texturesToRender.clear();
	normalsToRender.clear();
	tangentsToRender.clear();
	
	resourceMeshIndex = 0;

	for (auto component : componentsToRender)
	{
		if (component) //pointer not empty
		{
			AAA aaa = FindResourceMesh(component->GetMesh().get());
			ResourceMesh* resource = aaa.resourceMesh;
			verticesToRender.insert(std::end(verticesToRender), 
				std::begin(resource->GetVertices()), std::end(resource->GetVertices()));
			
			for (float3 tex : resource->GetTextureCoords())
			{
				texturesToRender.push_back(float2(tex.x, tex.y));
			}
			
			normalsToRender.insert(std::end(normalsToRender), 
				std::begin(resource->GetNormals()), std::end(resource->GetNormals()));

			if (flags & HAS_TANGENTS)
			{
				tangentsToRender.insert(std::end(tangentsToRender), 
					std::begin(resource->GetTangents()), std::end(resource->GetTangents()));
			}
			
			/*const float4x4& model =
				static_cast<ComponentTransform*>(component->GetOwner()
					->GetComponent(ComponentType::TRANSFORM))->GetGlobalMatrix();
*/


			//glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_TRUE, (const float*)&model);

			//do a for for all the instaces existing
			Command newCommand = { 
				resource->GetNumIndexes(),	// Number of indices in the mesh
				1,							// Number of instances to render
				aaa.indexOffset,			// Index offset in the EBO
				aaa.vertexOffset,			// Vertex offset in the VBO
				resourceMeshIndex			// Instance Index
			};

			commands.push_back(newCommand);
			resourceMeshIndex++;
		}
	}
	UpdateVAO();
}

const GameObject* GeometryBatch::GetComponentOwner(const ResourceMesh* resourceMesh)
{
	for (ComponentMeshRenderer* component : components)
	{
		if (component->GetMesh().get() == resourceMesh)
		{
			return component->GetOwner();
		}
	}

	return nullptr;
}

bool GeometryBatch::IsUniqueResourceMesh(const ResourceMesh* resourceMesh)
{
	for (AAA aaa : resourceMeshes)
	{
		if (aaa.resourceMesh == resourceMesh)
		{
			return false;
		}
	}

	return true;
}

AAA GeometryBatch::FindResourceMesh(ResourceMesh* mesh)
{
	for (auto aaa : resourceMeshes)
	{
		if (aaa.resourceMesh == mesh)
		{
			return aaa;
		}
	}
	abort(); //TODO check how can do this
}

int GeometryBatch::CalculateSpaceInVBO()
{
							//position			//uv				//normal		
	unsigned vertexSize = (sizeof(float) * 3 + sizeof(float) * 2 + sizeof(float) * 3);

	//tangents
	if (flags & HAS_TANGENTS)
	{
		vertexSize += sizeof(float) * 3;
	}
	return vertexSize;
}

bool GeometryBatch::CleanUp()
{
	glDeleteBuffers(1,&vao);
	glDeleteBuffers(1,&ebo);
	glDeleteBuffers(1,&vbo);
	glDeleteBuffers(1, &indirectBuffer);
	glDeleteBuffers(1, &verticesBuffer);
	glDeleteBuffers(1, &textureBuffer);
	glDeleteBuffers(1, &normalsBuffer);
	glDeleteBuffers(1, &tangentsBuffer);

	return true;
}
