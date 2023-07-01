#include "StdAfx.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "FileSystem/Json.h"
#include "Application.h"
#include "FileSystem/ModuleFileSystem.h"
#include "ModuleResources.h"

ResourceMaterial::ResourceMaterial(UID resourceUID,
								   const std::string& fileName,
								   const std::string& assetsPath,
								   const std::string& libraryPath) :
	Resource(resourceUID, fileName, assetsPath, libraryPath)
{
}

ResourceMaterial::~ResourceMaterial()
{
	this->Unload();
}

void ResourceMaterial::SaveLoadOptions(Json& meta)
{
	SavePaths(meta);

	meta["shaderType"] = static_cast<int>(loadOptions.shaderType);
	meta["diffuseColor_x"] = static_cast<float>(loadOptions.diffuseColor.x);
	meta["diffuseColor_y"] = static_cast<float>(loadOptions.diffuseColor.y);
	meta["diffuseColor_z"] = static_cast<float>(loadOptions.diffuseColor.z);
	meta["diffuseColor_w"] = static_cast<float>(loadOptions.diffuseColor.w);
	meta["specularColor_x"] = static_cast<float>(loadOptions.specularColor.x);
	meta["specularColor_y"] = static_cast<float>(loadOptions.specularColor.y);
	meta["specularColor_z"] = static_cast<float>(loadOptions.specularColor.z);
	meta["normalStrength"] = static_cast<float>(loadOptions.normalStrength);
	meta["smoothness"] = static_cast<float>(loadOptions.smoothness);
	meta["metalness"] = static_cast<float>(loadOptions.metalness);
	meta["isTransparent"] = static_cast<bool>(loadOptions.isTransparent);
	meta["tilingx"] = static_cast<float>(loadOptions.tiling.x);
	meta["tilingy"] = static_cast<float>(loadOptions.tiling.y);
	meta["offsetx"] = static_cast<float>(loadOptions.offset.x);
	meta["offsety"] = static_cast<float>(loadOptions.offset.y);
}

void ResourceMaterial::LoadLoadOptions(Json& meta)
{
	loadOptions.shaderType = static_cast<int>(meta["shaderType"]);

	LoadPaths(meta);

	loadOptions.diffuseColor.x = static_cast<float>(meta["diffuseColor_x"]);
	loadOptions.diffuseColor.y = static_cast<float>(meta["diffuseColor_y"]);
	loadOptions.diffuseColor.z = static_cast<float>(meta["diffuseColor_z"]);
	loadOptions.diffuseColor.w = static_cast<float>(meta["diffuseColor_w"]);
	if (loadOptions.diffuseColor.w == 0.f)
	{
		loadOptions.diffuseColor.w = 1.0f;
	} 
	loadOptions.specularColor.x = static_cast<float>(meta["specularColor_x"]);
	loadOptions.specularColor.y = static_cast<float>(meta["specularColor_y"]);
	loadOptions.specularColor.z = static_cast<float>(meta["specularColor_z"]);
	loadOptions.normalStrength = static_cast<float>(meta["normalStrength"]);
	loadOptions.smoothness = static_cast<float>(meta["smoothness"]);
	loadOptions.metalness = static_cast<float>(meta["metalness"]);
	loadOptions.isTransparent = static_cast<bool>(meta["isTransparent"]);
	loadOptions.tiling.x = static_cast<float>(meta["tilingx"]);
	loadOptions.tiling.y = static_cast<float>(meta["tilingy"]);
	if (loadOptions.tiling.x == 0.f && loadOptions.tiling.y == 0.f)
	{
		loadOptions.tiling.x = 1.f;
		loadOptions.tiling.y = 1.f;
	}
	loadOptions.offset.x = static_cast<float>(meta["offsetx"]);
	loadOptions.offset.y = static_cast<float>(meta["offsety"]);
}

void ResourceMaterial::SavePaths(Json& meta)
{
	if (diffuse)
	{
		meta["DiffuseAssetPath"] = diffuse->GetAssetsPath().c_str();
	}
	else
	{
		meta["DiffuseAssetPath"] = "";
	}

	if (normal)
	{
		meta["NormalAssetPath"] = normal->GetAssetsPath().c_str();
	}
	else
	{
		meta["NormalAssetPath"] = "";
	}

	if (occlusion)
	{
		meta["OcclusionAssetPath"] = occlusion->GetAssetsPath().c_str();
	}
	else
	{
		meta["OcclusionAssetPath"] = "";
	}

	switch (loadOptions.shaderType)
	{
		case 0:

			if (metallic)
			{
				meta["SpecularAssetPath"] = metallic->GetAssetsPath().c_str();
			}
			else
			{
				meta["SpecularAssetPath"] = "";
			}

			break;
		case 1:

			if (specular)
			{
				meta["SpecularAssetPath"] = specular->GetAssetsPath().c_str();
			}
			else
			{
				meta["SpecularAssetPath"] = "";
			}

			break;
	}

	if (emission)
	{
		meta["EmissiveAssetPath"] = emission->GetAssetsPath().c_str();
	}
	else
	{
		meta["EmissiveAssetPath"] = "";
	}
}

void ResourceMaterial::LoadPaths(Json& meta)
{
	std::string assetPath = meta["DiffuseAssetPath"];
	ModuleResources* resources = App->GetModule<ModuleResources>();

	if (CheckAssetPath(assetPath))
	{
		SetDiffuse(resources->RequestResource<ResourceTexture>(assetPath));
	}

	assetPath = meta["NormalAssetPath"];

	if (CheckAssetPath(assetPath))
	{
		SetNormal(resources->RequestResource<ResourceTexture>(assetPath));
	}

	assetPath = meta["OcclusionAssetPath"];

	if (CheckAssetPath(assetPath))
	{
		SetOcclusion(resources->RequestResource<ResourceTexture>(assetPath));
	}

	assetPath = meta["Metalic/SpecularAssetPath"];

	if (CheckAssetPath(assetPath))
	{
		switch (loadOptions.shaderType)
		{
			case 0:
				SetMetallic(resources->RequestResource<ResourceTexture>(assetPath));
				break;

			case 1:
				SetSpecular(resources->RequestResource<ResourceTexture>(assetPath));
				break;
		}
	}

	assetPath = meta["EmissiveAssetPath"];

	if (CheckAssetPath(assetPath))
	{
		SetEmission(resources->RequestResource<ResourceTexture>(assetPath));
	}
}

bool ResourceMaterial::CheckAssetPath(std::string& assetPath)
{
	if (assetPath != "")
	{
		bool materialExists = assetPath != "" && App->GetModule<ModuleFileSystem>()->Exists(assetPath.c_str());
		if (materialExists)
		{
			return true;
		}
	}
	return false;
}
