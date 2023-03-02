#include "WindowComponentMaterial.h"

#include "DataModels/Components/ComponentMaterial.h"

#include "Application.h"
#include "FileSystem/ModuleResources.h"

#include "DataModels/Windows/EditorWindows/ImporterWindows/WindowTextureInput.h"
#include "DataModels/Windows/EditorWindows/ImporterWindows/WindowMaterialInput.h"

#include "DataModels/Resources/ResourceMaterial.h"
#include "DataModels/Resources/ResourceTexture.h"

WindowComponentMaterial::WindowComponentMaterial(ComponentMaterial* component) :
	ComponentWindow("MATERIAL", component), 
	inputMaterial(std::make_unique<WindowMaterialInput>(component)),
	inputTextureDiffuse(std::make_unique<WindowTextureInput>(component, TextureType::DIFFUSE)),
	inputTextureNormal(std::make_unique<WindowTextureInput>(component, TextureType::NORMAL)),
	inputTextureSpecular(std::make_unique<WindowTextureInput>(component, TextureType::SPECULAR))
{
}

WindowComponentMaterial::~WindowComponentMaterial()
{
}

void WindowComponentMaterial::DrawWindowContents()
{
	DrawEnableAndDeleteComponent();

	ComponentMaterial* asMaterial = static_cast<ComponentMaterial*>(component);

	if (asMaterial)
	{
		if (asMaterial->GetMaterial())
		{
			DrawSetMaterial();
		}
		else
		{
			DrawEmptyMaterial();
		}
	}
}

void WindowComponentMaterial::DrawSetMaterial()
{
	ComponentMaterial* asMaterial = static_cast<ComponentMaterial*>(component);

	if (asMaterial)
	{
		std::shared_ptr<ResourceMaterial> materialResource = asMaterial->GetMaterial();
		if (materialResource)
		{
			ImGui::Text("");

			char name[20] = "Texture";
			ImGui::InputText("Texture Name", name, 20);

			ImGui::Text("");

			static float3 colorDiffuse = asMaterial->GetDiffuseColor();
			ImGui::Text("Diffuse Color:"); ImGui::SameLine();
			if (ImGui::ColorEdit3("##Diffuse Color", (float*)&colorDiffuse))
			{
				asMaterial->SetDiffuseColor(colorDiffuse);
			}

			static float3 colorSpecular = asMaterial->GetSpecularColor();
			ImGui::Text("Specular Color:"); ImGui::SameLine();
			if (ImGui::ColorEdit3("##Specular Color", (float*)&colorSpecular))
			{
				asMaterial->SetSpecularColor(colorSpecular);
			}

			ImGui::Text("");

			static UID thisUID = UniqueID::GenerateUID();

			std::string removeButtonLabel = "No Texture";

			if (materialResource)
			{
				if (materialResource->GetDiffuse() || materialResource->GetNormal()
					|| materialResource->GetSpecular())
				{
					removeButtonLabel = "Remove Textures";
				}

			}

			if (ImGui::Button(removeButtonLabel.c_str()) && materialResource)
			{
				asMaterial->UnloadTextures();
				
				materialResource->SetDiffuse(nullptr);
				materialResource->SetNormal(nullptr);
				materialResource->SetOcclusion(nullptr);
				materialResource->SetSpecular(nullptr);
				
				materialResource->SetChanged(true);
			}

			bool hasShininessAlpha = asMaterial->HasShininessAlpha();
			ImGui::Checkbox("Use specular Alpha as shininess", &hasShininessAlpha);
			asMaterial->SetHasShininessAlpha(hasShininessAlpha);

			float shininess = asMaterial->GetShininess();
			ImGui::SliderFloat("Shininess", &shininess,
				0.1f, 200.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
			asMaterial->SetShininess(shininess);
			ImGui::Separator();

			ImGui::Text("Diffuse Texture");
			bool showTextureBrowserDiffuse = true;
			std::shared_ptr<ResourceTexture> texture;
			if (materialResource)
			{
				if (materialResource->GetDiffuse())
				{
					texture = materialResource->GetDiffuse();
					if (texture)
					{
						ImGui::Image((void*)(intptr_t)texture->GetGlTexture(), ImVec2(100, 100));
					}

					showTextureBrowserDiffuse = false;
				}
			}

			if (showTextureBrowserDiffuse)
			{
				inputTextureDiffuse->DrawWindowContents();
			}
			else
			{
				if (ImGui::Button("Remove Texture Diffuse") && materialResource->GetDiffuse())
				{
					asMaterial->UnloadTexture(TextureType::DIFFUSE);

					materialResource->SetDiffuse(nullptr);
				}
			}

			ImGui::Separator();

			ImGui::Text("Specular Texture");
			bool showTextureBrowserSpecular = true;
			if (materialResource && materialResource->GetSpecular())
			{
				texture =
					std::dynamic_pointer_cast<ResourceTexture>(materialResource->GetSpecular());
				if (texture)
				{
					ImGui::Image((void*)(intptr_t)texture->GetGlTexture(), ImVec2(100, 100));
				}

				showTextureBrowserSpecular = false;
			}

			if (showTextureBrowserSpecular)
			{
				inputTextureSpecular->DrawWindowContents();
			}
			else
			{
				if (ImGui::Button("Remove Texture Specular") && materialResource->GetSpecular())
				{
					asMaterial->UnloadTexture(TextureType::SPECULAR);

					materialResource->SetSpecular(nullptr);
				}
			}

			ImGui::Separator();

			ImGui::Text("Normal Texture");
			bool showTextureBrowserNormal = true;
			if (materialResource && materialResource->GetNormal())
			{
					texture = materialResource->GetNormal();
					if (texture)
					{
						ImGui::Image((void*)(intptr_t)texture->GetGlTexture(), ImVec2(100, 100));
					}

					showTextureBrowserNormal = false;
			}

			if (showTextureBrowserNormal)
			{
				inputTextureNormal->DrawWindowContents();
			}
			else if (ImGui::Button("Remove Texture Normal"))
			{
				if (materialResource->GetNormal())
				{
					asMaterial->UnloadTexture(TextureType::NORMAL);

					materialResource->SetNormal(nullptr);
				}
			}
			float normalStrength = asMaterial->GetNormalStrenght();
			ImGui::SliderFloat("Normal", &normalStrength,
				0.0f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
			asMaterial->SetNormalStrenght(normalStrength);

			ImGui::Text("");
		}
	}
}

void WindowComponentMaterial::DrawEmptyMaterial()
{
	ComponentMaterial* asMaterial = static_cast<ComponentMaterial*>(component);

	if (asMaterial)
	{
		if (asMaterial->GetMaterial() == nullptr)
		{
			inputMaterial->DrawWindowContents();
		}
	}
}
