#include "StdAfx.h"

#include "ComponentMeshRenderer.h"

#include "ComponentTransform.h"

#include "Application.h"

#include "FileSystem/Json.h"
#include "FileSystem/ModuleFileSystem.h"
#include "FileSystem/ModuleResources.h"
#include "ModuleCamera.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"

#include "Program/Program.h"

#include "Resources/ResourceMaterial.h"
#include "Resources/ResourceMesh.h"
#include "Resources/ResourceTexture.h"

#include "Batch/BatchManager.h"
#include "Batch/GeometryBatch.h"

#include "GameObject/GameObject.h"

#include "Camera/Camera.h"

#include "Enums/TextureType.h"

#include <GL/glew.h>

#ifdef ENGINE
	#include "DataModels/Resources/EditorResource/EditorResourceInterface.h"
#else
	#include "Modules/ModuleEditor.h"
	#include "Windows/WindowDebug.h"
#endif

ComponentMeshRenderer::ComponentMeshRenderer(const bool active, GameObject* owner) :
	Component(ComponentType::MESHRENDERER, active, owner, true), effectColor(0.f, 0.f, 0.f, 0.f), discard(false)
{
}

ComponentMeshRenderer::ComponentMeshRenderer(const ComponentMeshRenderer& componentMeshRenderer) :
	Component(componentMeshRenderer),
	material(componentMeshRenderer.GetMaterial()), effectColor(0.f, 0.f, 0.f, 0.f), discard(false)
{
	SetOwner(componentMeshRenderer.GetOwner());
	SetMesh(componentMeshRenderer.GetMesh());
}

ComponentMeshRenderer::~ComponentMeshRenderer()
{
	if (mesh)
	{
		if (batch)
		{
			batch->DeleteComponent(this);
		}
		mesh->Unload();
	}
	if (material)
	{
		material->Unload();
	}
}

void ComponentMeshRenderer::InitBones()
{
	const unsigned int numBones = mesh->GetNumBones();

	skinPalette.resize(numBones);

	for (unsigned int i = 0; i < numBones; ++i)
	{
		skinPalette[i] = float4x4::identity;
	}
}

void ComponentMeshRenderer::UpdatePalette()
{
	if (mesh && mesh->GetNumBones() > 0)
	{
		GameObject* root = GetOwner()->GetRootGO();

		if (root)
		{
			const std::vector<Bone>& bindBones = mesh->GetBones();

			for (unsigned int i = 0; i < mesh->GetNumBones(); ++i)
			{
				const GameObject* boneNode = root->FindGameObject(bindBones[i].name);

				if (boneNode && App->GetPlayState() == Application::PlayState::RUNNING)
				{
					skinPalette[i] = boneNode->GetComponentInternal<ComponentTransform>()->CalculatePaletteGlobalMatrix() *
									 bindBones[i].transform;
				}
				else
				{
					skinPalette[i] = float4x4::identity;
				}
			}
		}
	}
}

void ComponentMeshRenderer::Draw() const
{
	/*if (material)
	{
		Program* program = App->GetModule<ModuleProgram>()->GetProgram(ProgramType(material->GetShaderType()));

		if (program)
		{
			program->Activate();

			DrawMaterial(program);
			DrawMeshes(program);
		}

		program->Deactivate();
	}*/
	ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();
	if (transform == nullptr)
	{
		return;
	}
#ifndef ENGINE
	if (App->GetModule<ModuleEditor>()->GetDebugOptions()->GetDrawBoundingBoxes())
	{
		App->GetModule<ModuleDebugDraw>()->DrawBoundingBox(transform->GetObjectOBB());
	}
#endif // ENGINE
	if (transform->IsDrawBoundingBoxes())
	{
		App->GetModule<ModuleDebugDraw>()->DrawBoundingBox(transform->GetObjectOBB());
	}
}

void ComponentMeshRenderer::DrawMeshes(Program* program) const
{
#ifdef ENGINE

	// this should be in an EditorComponent class, or something of the like
	// but for now have it here
	if (mesh && std::dynamic_pointer_cast<EditorResourceInterface>(mesh)->ToDelete())
	{
		mesh = nullptr;
	}

#endif

	if (!mesh)
	{
		return;
	}

	if (!mesh->IsLoaded())
	{
		mesh->Load();
	}

	// --------- Bones -----------
	int hasBones = 0;
	if (!skinPalette.empty())
	{
		hasBones = 1;
	}

	program->BindUniformInt("hasBones", hasBones);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh->GetSSBOPalette());
	if (hasBones)
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float4x4) * skinPalette.size(), &skinPalette[0]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, mesh->GetSSBOPalette());
	}
	// ---------------------------

	const float4x4& view = App->GetModule<ModuleCamera>()->GetCamera()->GetViewMatrix();
	const float4x4& proj = App->GetModule<ModuleCamera>()->GetCamera()->GetProjectionMatrix();
	const float4x4& model = GetOwner()->GetComponentInternal<ComponentTransform>()->GetGlobalMatrix();

	glUniformMatrix4fv(2, 1, GL_TRUE, (const float*) &model);
	glUniformMatrix4fv(1, 1, GL_TRUE, (const float*) &view);
	glUniformMatrix4fv(0, 1, GL_TRUE, (const float*) &proj);

	glBindVertexArray(mesh->GetVAO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetEBO());

	glDrawElements(GL_TRIANGLES, mesh->GetNumFaces() * 3, GL_UNSIGNED_INT, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComponentMeshRenderer::DrawMaterial(Program* program) const
{
#ifdef ENGINE

	// this should be in an EditorComponent class, or something of the like
	// but for now have it here
	if (material && std::dynamic_pointer_cast<EditorResourceInterface>(material)->ToDelete())
	{
		material = nullptr;
	}

#endif

	if (material)
	{
		const float4& diffuseColor = material->GetDiffuseColor();

		glUniform4f(3, diffuseColor.x, diffuseColor.y, diffuseColor.z, diffuseColor.w);

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

		texture = std::dynamic_pointer_cast<ResourceTexture>(material->GetNormal());

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

		switch (material->GetShaderType())
		{
			case 0:

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

			case 1:

				const float3& specularColor = material->GetSpecularColor();
				glUniform3f(8, specularColor.x, specularColor.y, specularColor.z);

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

		texture = material->GetEmission();
		if (texture)
		{
			if (!texture->IsLoaded())
			{
				texture->Load();
			}

			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, texture->GetGlTexture());
			glUniform1i(10, 1);
		}
		else
		{
			glUniform1i(10, 0);
		}

		float3 viewPos = App->GetModule<ModuleCamera>()->GetCamera()->GetPosition();
		program->BindUniformFloat3("viewPos", viewPos);
	}
}

void ComponentMeshRenderer::DrawHighlight() const
{
	if (!mesh)
	{
		return;
	}
	if (!mesh->IsLoaded())
	{
		mesh->Load();
	}

	float scale = 10.1f;
	Program* program = App->GetModule<ModuleProgram>()->GetProgram(ProgramType::HIGHLIGHT);

	if (program)
	{
		program->Activate();
		const float4x4& view = App->GetModule<ModuleCamera>()->GetCamera()->GetViewMatrix();
		const float4x4& proj = App->GetModule<ModuleCamera>()->GetCamera()->GetProjectionMatrix();
		const float4x4& model = GetOwner()->GetComponentInternal<ComponentTransform>()->GetGlobalMatrix();

		GLint programInUse;

		glGetIntegerv(GL_CURRENT_PROGRAM, &programInUse);

		glUniformMatrix4fv(2, 1, GL_TRUE, (const float*) &model);
		glUniformMatrix4fv(1, 1, GL_TRUE, (const float*) &view);
		glUniformMatrix4fv(0, 1, GL_TRUE, (const float*) &proj);

		glBindVertexArray(mesh->GetVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetEBO());

		glDrawElements(GL_TRIANGLES, mesh->GetNumFaces() * 3, GL_UNSIGNED_INT, nullptr);

		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		program->Deactivate();
	}
}

void ComponentMeshRenderer::InternalSave(Json& meta)
{
	UID uid = 0;
	std::string assetPath = "";

	if (mesh)
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

void ComponentMeshRenderer::InternalLoad(const Json& meta)
{
#ifdef ENGINE
	std::string path = meta["assetPathMaterial"];
	bool materialExists = !path.empty() && App->GetModule<ModuleFileSystem>()->Exists(path.c_str());

	if (materialExists)
	{
		std::shared_ptr<ResourceMaterial> resourceMaterial =
			App->GetModule<ModuleResources>()->RequestResource<ResourceMaterial>(path);

		if (resourceMaterial)
		{
			SetMaterial(resourceMaterial);
		}
	}
	path = meta["assetPathMesh"];
	bool meshExists = !path.empty() && App->GetModule<ModuleFileSystem>()->Exists(path.c_str());

	if (meshExists)
	{
		std::shared_ptr<ResourceMesh> resourceMesh =
			App->GetModule<ModuleResources>()->RequestResource<ResourceMesh>(path);

		if (resourceMesh)
		{
			SetMesh(resourceMesh);
		}
	}
#else

	UID uidMaterial = meta["materialUID"];
	std::shared_ptr<ResourceMaterial> resourceMaterial =
		App->GetModule<ModuleResources>()->SearchResource<ResourceMaterial>(uidMaterial);

	if (resourceMaterial)
	{
		SetMaterial(resourceMaterial);
	}

	UID uidMesh = meta["meshUID"];
	std::shared_ptr<ResourceMesh> resourceMesh =
		App->GetModule<ModuleResources>()->SearchResource<ResourceMesh>(uidMesh);

	if (resourceMesh)
	{
		SetMesh(resourceMesh);
	}

#endif
}
void ComponentMeshRenderer::SetMesh(const std::shared_ptr<ResourceMesh>& newMesh)
{
	mesh = newMesh;

	if (mesh)
	{
		mesh->Load();

		ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();

		transform->Encapsule(mesh->GetVertices().data(), mesh->GetNumVertices());
		//set the origin to translate and scale the BoundingBox
		transform->SetOriginScaling(transform->GetLocalAABB().HalfSize());
		transform->SetOriginCenter(transform->GetLocalAABB().CenterPoint());

		//Apply the BoundingBox modification
		float3 translation = transform->GetBBPos();
		float3 scaling = transform->GetBBScale();
		transform->TranslateLocalAABB(translation);
		transform->ScaleLocalAABB(scaling);
		transform->CalculateBoundingBoxes();

		App->GetModule<ModuleRender>()->GetBatchManager()->AddComponent(this);

		InitBones();
	}
	else
	{
		batch->DeleteComponent(this);
		batch = nullptr;
	}
}

void ComponentMeshRenderer::SetMaterial(const std::shared_ptr<ResourceMaterial>& newMaterial)
{
	material = newMaterial;

	if (material)
	{
		material->Load();
	}
}

void ComponentMeshRenderer::UnloadTextures()
{
	if (material)
	{
		std::shared_ptr<ResourceTexture> texture = material->GetDiffuse();
		if (texture)
		{
			texture->Unload();
		}

		texture = material->GetNormal();
		if (texture)
		{
			texture->Unload();
		}

		texture = material->GetOcclusion();
		if (texture)
		{
			texture->Unload();
		}

		texture = material->GetSpecular();
		if (texture)
		{
			texture->Unload();
		}

		texture = material->GetMetallic();
		if (texture)
		{
			texture->Unload();
		}

		texture = material->GetEmission();
		if (texture)
		{
			texture->Unload();
		}
	}
}

void ComponentMeshRenderer::UnloadTexture(TextureType textureType)
{
	if (material)
	{
		std::shared_ptr<ResourceTexture> texture;
		switch (textureType)
		{
		case TextureType::DIFFUSE:
			texture = material->GetDiffuse();
			if (texture)
			{
				texture->Unload();
			}
			break;
		case TextureType::NORMAL:
			texture = material->GetNormal();
			if (texture)
			{
				texture->Unload();
			}
			break;
		case TextureType::OCCLUSION:
			texture = material->GetOcclusion();
			if (texture)
			{
				texture->Unload();
			}
			break;
		case TextureType::SPECULAR:
			texture = material->GetSpecular();
			if (texture)
			{
				texture->Unload();
			}
			break;
		case TextureType::METALLIC:
			texture = material->GetMetallic();
			if (texture)
			{
				texture->Unload();
			}
			break;
		case TextureType::EMISSION:
			texture = material->GetEmission();
			if (texture)
			{
				texture->Unload();
			}
			break;
		}
	}
}

// Common attributes (setters)
void ComponentMeshRenderer::SetDiffuseColor(float4& diffuseColor)
{
	material->SetDiffuseColor(diffuseColor);
}

void ComponentMeshRenderer::SetDiffuse(const std::shared_ptr<ResourceTexture>& diffuse)
{
	material->SetDiffuse(diffuse);
}

void ComponentMeshRenderer::SetNormal(const std::shared_ptr<ResourceTexture>& normal)
{
	material->SetNormal(normal);
}

void ComponentMeshRenderer::SetMetallic(const std::shared_ptr<ResourceTexture>& metallic)
{
	material->SetMetallic(metallic);
}

void ComponentMeshRenderer::SetSpecular(const std::shared_ptr<ResourceTexture>& specular)
{
	material->SetSpecular(specular);
}

void ComponentMeshRenderer::SetEmissive(const std::shared_ptr<ResourceTexture>& emissive)
{
	material->SetEmission(emissive);
}

void ComponentMeshRenderer::SetShaderType(unsigned int shaderType)
{
	material->SetShaderType(shaderType);
}

void ComponentMeshRenderer::SetSmoothness(float smoothness)
{
	material->SetSmoothness(smoothness);
}

void ComponentMeshRenderer::SetNormalStrength(float normalStrength)
{
	material->SetNormalStrength(normalStrength);
}

void ComponentMeshRenderer::SetTiling(const float2& tiling)
{
	material->SetTiling(tiling);
}

void ComponentMeshRenderer::SetOffset(const float2& offset)
{
	material->SetOffset(offset);
}

const float4& ComponentMeshRenderer::GetEffectColor() const
{
	return effectColor;
}

void ComponentMeshRenderer::SetEffectColor(float4 effectColor)
{
	this->effectColor = effectColor;
}

bool ComponentMeshRenderer::IsDiscarded()
{
	return discard;
}

void ComponentMeshRenderer::SetDiscard(bool discard)
{
	this->discard = discard;
}

// Default shader attributes (setters)
void ComponentMeshRenderer::SetMetalness(float metalness)
{
	material->SetMetalness(metalness);
}

// Specular shader attributes (setters)
void ComponentMeshRenderer::SetSpecularColor(float3& specularColor)
{
	material->SetSpecularColor(specularColor);
}

void ComponentMeshRenderer::SetTransparent(bool isTransparent)
{
	material->SetTransparent(isTransparent);
}

void ComponentMeshRenderer::RemoveFromBatch()
{
	batch->DeleteComponent(this);
}

std::vector<ComponentMeshRenderer*> ComponentMeshRenderer::ChangeOfBatch()
{
	return batch->ChangeBatch(this);
}

const unsigned int& ComponentMeshRenderer::GetShaderType() const
{
	return material->GetShaderType();
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

const bool ComponentMeshRenderer::IsReflective() const
{
	return material->IsReflective();
}

void ComponentMeshRenderer::SetReflective(bool reflective) const
{
	material->SetReflective(reflective);
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

const bool ComponentMeshRenderer::IsTransparent() const
{
	return material->IsTransparent();
}

std::shared_ptr<ResourceTexture> ComponentMeshRenderer::GetDiffuse() const
{
	return material->GetDiffuse();
}

std::shared_ptr<ResourceTexture> ComponentMeshRenderer::GetNormal() const
{
	return material->GetNormal();
}

std::shared_ptr<ResourceTexture> ComponentMeshRenderer::GetOcclusion() const
{
	return material->GetOcclusion();
}

std::shared_ptr<ResourceTexture> ComponentMeshRenderer::GetMetallic() const
{
	return material->GetMetallic();
}

std::shared_ptr<ResourceTexture> ComponentMeshRenderer::GetSpecular() const
{
	return material->GetSpecular();
}
