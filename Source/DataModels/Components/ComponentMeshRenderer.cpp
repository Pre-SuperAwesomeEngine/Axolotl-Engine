#pragma warning (disable: 26495)
#pragma warning (disable: 4804)
#pragma warning (disable: 4312)

#include "ComponentMeshRenderer.h"

#include "ComponentTransform.h"

#include "Application.h"

#include "ModuleCamera.h"
#include "Program/Program.h"
#include "FileSystem/ModuleResources.h"
#include "FileSystem/ModuleFileSystem.h"
#include "FileSystem/Json.h"

#include "Resources/ResourceMesh.h"
#include "Resources/ResourceMaterial.h"
#include "Resources/ResourceTexture.h"

#include "GameObject/GameObject.h"

#include <GL/glew.h>


#ifdef ENGINE

#include "DataModels/Resources/EditorResource/EditorResourceInterface.h"

#endif

ComponentMeshRenderer::ComponentMeshRenderer(const bool active, GameObject* owner)
	: Component(ComponentType::MESHRENDERER, active, owner, true), 
	shaderType(ProgramType::DEFAULT)
{
}

ComponentMeshRenderer::ComponentMeshRenderer(const ComponentMeshRenderer& componentMeshRenderer):
	Component(componentMeshRenderer), 
	mesh(componentMeshRenderer.GetMesh()), material(componentMeshRenderer.GetMaterial())
{
}

ComponentMeshRenderer::~ComponentMeshRenderer()
{
	if (mesh)
		mesh->Unload();
}

void ComponentMeshRenderer::Update()
{

}

void ComponentMeshRenderer::Draw()
{
	Program* program = App->program->GetProgram(shaderType);

	if (program)
	{
		program->Activate();

		DrawMaterial(program);
		DrawMeshes(program);
	}

	program->Deactivate();
}
void ComponentMeshRenderer::DrawMeshes(Program* program)
{

#ifdef ENGINE

	//this should be in an EditorComponent class, or something of the like
	//but for now have it here
	if (mesh && 
		std::dynamic_pointer_cast<EditorResourceInterface>(mesh)->ToDelete())
	{
		mesh = nullptr;
	}

#endif

	if (this->IsMeshLoaded())
	{
		if (!mesh->IsLoaded())
		{
			mesh->Load();
		}
		
		const float4x4& view = App->camera->GetCamera()->GetViewMatrix();
		const float4x4& proj = App->camera->GetCamera()->GetProjectionMatrix();
		const float4x4& model =
			static_cast<ComponentTransform*>(GetOwner()
				->GetComponent(ComponentType::TRANSFORM))->GetGlobalMatrix();

		glUniformMatrix4fv(2, 1, GL_TRUE, (const float*)&model);
		glUniformMatrix4fv(1, 1, GL_TRUE, (const float*)&view);
		glUniformMatrix4fv(0, 1, GL_TRUE, (const float*)&proj);

		glBindVertexArray(mesh->GetVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetEBO());

		glDrawElements(GL_TRIANGLES, mesh->GetNumFaces() * 3, 
												GL_UNSIGNED_INT, nullptr);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void ComponentMeshRenderer::DrawMaterial(Program* program)
{

#ifdef ENGINE

	//this should be in an EditorComponent class, or something of the like
	//but for now have it here
	if (material && 
		std::dynamic_pointer_cast<EditorResourceInterface>
													(material)->ToDelete())
	{
		material = nullptr;
	}

#endif

	if (material)
	{
		const float4& diffuseColor = material->GetDiffuseColor();

		glUniform4f(3, diffuseColor.x, diffuseColor.y, diffuseColor.z, 
															diffuseColor.w);

		std::shared_ptr<ResourceTexture> texture = material->GetDiffuse();

		if (texture)
		{
			if (!texture->IsLoaded())
			{
				texture->Load();
			}

			glUniform1i(4, 1);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
		}
		else
		{
			glUniform1i(4, 0);
		}

		glUniform1f(5, material->GetSmoothness());
		glUniform1f(6, material->GetNormalStrength());

		texture =
			std::dynamic_pointer_cast<ResourceTexture>(material->GetNormal());

		if (texture)
		{
			if (!texture->IsLoaded())
			{
				texture->Load();
			}

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
			glUniform1i(7, 1);
		}
		else
		{
			glUniform1i(7, 0);
		}

		switch (shaderType)
		{
			case ProgramType::DEFAULT:

				glUniform1f(8, material->GetMetalness());

				texture = material->GetMetallic();
				if (texture)
				{
					if (!texture->IsLoaded())
					{
						texture->Load();
					}

					glUniform1i(9, 1);
					glActiveTexture(GL_TEXTURE7);
					glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
				}
				else
				{
					glUniform1i(9, 0);
				}

				break;

			case ProgramType::SPECULAR:

				const float3& specularColor = material->GetSpecularColor();
				glUniform3f
						(8, specularColor.x, specularColor.y, specularColor.z);

				texture = material->GetSpecular();

				if (texture)
				{
					if (!texture->IsLoaded())
					{
						texture->Load();
					}

					glUniform1i(9, 1);
					glActiveTexture(GL_TEXTURE7);
					glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
				}
				else
				{
					glUniform1i(9, 0);
				}

				break;
		}

		float3 viewPos = App->camera->GetCamera()->GetPosition();
		program->BindUniformFloat3("viewPos", viewPos);
	}
}

void ComponentMeshRenderer::DrawHighlight()
{
	if (IsMeshLoaded())
	{
		if (!mesh->IsLoaded())
		{
			mesh->Load();
		}

		float scale = 10.1f;
		Program* program = App->program->GetProgram(ProgramType::HIGHLIGHT);

		if (program)
		{
			program->Activate();
			const float4x4& view = App->camera->GetCamera()->GetViewMatrix();
			const float4x4& proj = 
							App->camera->GetCamera()->GetProjectionMatrix();
			const float4x4& model =
				static_cast<ComponentTransform*>(GetOwner()
					->GetComponent(ComponentType::TRANSFORM))
														->GetGlobalMatrix();

			GLint programInUse;

			glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

			glUniformMatrix4fv(2, 1, GL_TRUE, (const float*)&model);
			glUniformMatrix4fv(1, 1, GL_TRUE, (const float*)&view);
			glUniformMatrix4fv(0, 1, GL_TRUE, (const float*)&proj);

			glBindVertexArray(mesh->GetVAO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetEBO());

			glDrawElements(GL_TRIANGLES, mesh->GetNumFaces() * 3, 
													GL_UNSIGNED_INT, nullptr);

			glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			program->Deactivate();
		}
	}
}

void ComponentMeshRenderer::SaveOptions(Json& meta)
{
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = static_cast<bool>(active);
	meta["removed"] = static_cast<bool>(canBeRemoved);

	UID uid = 0;
	std::string assetPath = "";

	if(mesh)
	{
		uid = mesh->GetUID();
		assetPath = mesh->GetAssetsPath();
	}

	meta["meshUID"] = static_cast<UID>(uid);
	meta["assetPathMesh"] = assetPath.c_str();

	if (material)
	{
		uid = material->GetUID();
		assetPath = material->GetAssetsPath();
	}

	meta["materialUID"] = static_cast<UID>(uid);
	meta["assetPathMaterial"] = assetPath.c_str();
}

void ComponentMeshRenderer::LoadOptions(Json& meta)
{
	type = GetTypeByName(meta["type"]);
	active = static_cast<bool>(meta["active"]);
	canBeRemoved = static_cast<bool>(meta["removed"]);

#ifdef ENGINE

	std::string path = meta["assetPathMesh"];
	bool meshExists = path != "" && App->fileSystem->Exists(path.c_str());

	if (meshExists)
	{
		std::shared_ptr<ResourceMesh> resourceMesh = 
			App->resources->RequestResource<ResourceMesh>(path);

		if (resourceMesh)
		{
			SetMesh(resourceMesh);
		}
	}

	path = meta["assetPathMaterial"];
	bool materialExists = path != "" && App->fileSystem->Exists(path.c_str());

	if (materialExists)
	{
		std::shared_ptr<ResourceMaterial> resourceMaterial = 
			App->resources->RequestResource<ResourceMaterial>(path);

		if (resourceMaterial)
		{
			SetMaterial(resourceMaterial);
		}
	}
#else

	UID uidMesh = meta["meshUID"];
	std::shared_ptr<ResourceMesh> resourceMesh = 
		App->resources->SearchResource<ResourceMesh>(uidMesh);

	if (resourceMesh)
	{
		SetMesh(resourceMesh);
	}

	UID uidMaterial = meta["materialUID"];
	std::shared_ptr<ResourceMaterial> resourceMaterial = 
		App->resources->SearchResource<ResourceMaterial>(uidMaterial);

	if (resourceMaterial)
	{
		SetMaterial(resourceMaterial);
	}

#endif
}

void ComponentMeshRenderer::SetMesh
			(const std::shared_ptr<ResourceMesh>& newMesh)
{
	mesh = newMesh;

	if (mesh)
	{
		mesh->Load();

		ComponentTransform* transform = 
			static_cast<ComponentTransform*>
			(GetOwner()->GetComponent(ComponentType::TRANSFORM));
		
		transform->Encapsule
				(mesh->GetVertices().data(), mesh->GetNumVertices());
	}
}

void ComponentMeshRenderer::SetMaterial
			(const std::shared_ptr<ResourceMaterial>& newMaterial)
{
	material = newMaterial;

	if (material)
	{
		material->Load();
	}
}

void ComponentMeshRenderer::SetShaderType(const unsigned shaderType)
{
	switch (shaderType)
	{
		case 0:

			this->shaderType = ProgramType::DEFAULT;

			break;

		case 1:

			this->shaderType = ProgramType::SPECULAR;

			break;
	}
}

void ComponentMeshRenderer::UnloadTextures()
{
	if (material)
	{
		if (material->GetDiffuse())
		{
			material->GetDiffuse()->Unload();
		}

		if (material->GetNormal())
		{
			material->GetNormal()->Unload();
		}

		if (material->GetOcclusion())
		{
			material->GetOcclusion()->Unload();
		}

		if (material->GetMetallic())
		{
			material->GetMetallic()->Unload();
		}

		if (material->GetSpecular())
		{
			material->GetSpecular()->Unload();
		}
	}
}

void ComponentMeshRenderer::UnloadTexture(TextureType textureType)
{
	if (material)
	{
		switch (textureType)
		{
			case TextureType::DIFFUSE:

				if (material->GetDiffuse())
				{
					material->GetDiffuse()->Unload();
				}

				break;

			case TextureType::NORMAL:

				if (material->GetNormal())
				{
					material->GetNormal()->Unload();
				}

				break;

			case TextureType::OCCLUSION:

				if (material->GetOcclusion())
				{
					material->GetOcclusion()->Unload();
				}

				break;

			case TextureType::METALLIC:
				
				if (material->GetMetallic())
				{
					material->GetMetallic()->Unload();
				}

				break;

			case TextureType::SPECULAR:

				if (material->GetSpecular())
				{
					material->GetSpecular()->Unload();
				}

				break;

		}
	}
}

// Common attributes (setters)

void ComponentMeshRenderer::SetDiffuseColor(float4& diffuseColor)
{
	this->material->SetDiffuseColor(diffuseColor);
}

void ComponentMeshRenderer::SetSmoothness(float smoothness)
{
	this->material->SetSmoothness(smoothness);
}

void ComponentMeshRenderer::SetNormalStrenght(float normalStrength)
{
	this->material->SetNormalStrength(normalStrength);
}

// Default shader attributes (setters)

void ComponentMeshRenderer::SetMetalness(float metalness)
{
	this->material->SetMetalness(metalness);
}

// Specular shader attributes (setters)

void ComponentMeshRenderer::SetSpecularColor(float3& specularColor)
{
	this->material->SetSpecularColor(specularColor);
}

// Common attributes (getters)

const float4& ComponentMeshRenderer::GetDiffuseColor() const
{
	return material->GetDiffuseColor();
}

const float ComponentMeshRenderer::GetSmoothness() const
{
	return material->GetSmoothness();
}

const float ComponentMeshRenderer::GetNormalStrenght() const
{
	return material->GetNormalStrength();
}

// Default shader attributes (getters)

const float ComponentMeshRenderer::GetMetalness() const
{
	return material->GetMetalness();
}

// Specular shader attributes (getters)

const float3& ComponentMeshRenderer::GetSpecularColor() const
{
	return material->GetSpecularColor();
}