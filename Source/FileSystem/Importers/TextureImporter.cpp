#include "StdAfx.h"

#include "TextureImporter.h"

#include "Application.h"
#include "FileSystem/Json.h"
#include "FileSystem/ModuleFileSystem.h"
#include "Resources/ResourceTexture.h"

#include "Defines/ExtensionDefines.h"

#include <DirectXTex/DirectXTex.h>
#include <GL/glew.h>

#define DDS_TEXTURE_EXTENSION ".dds"

TextureImporter::TextureImporter()
{
}

TextureImporter::~TextureImporter()
{
}

void TextureImporter::Import(const char* filePath, std::shared_ptr<ResourceTexture> resource)
{
	LOG_VERBOSE("Import texture from {}", filePath);

	ImportOptionsTexture options = resource->GetImportOptions();

	std::string narrowString(filePath);
	std::wstring wideString = std::wstring(narrowString.begin(), narrowString.end());
	const wchar_t* path = wideString.c_str();

	DirectX::TexMetadata md;
	DirectX::ScratchImage* imgResult;
	DirectX::ScratchImage img, flippedImg, dcmprsdImg;

	HRESULT result = DirectX::LoadFromDDSFile(path, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &md, img);

	if (FAILED(result)) // DDS failed try with TGA
	{
		result = DirectX::LoadFromTGAFile(path, &md, img);

		if (FAILED(result)) // TGA failed try with WIC
		{
			result = DirectX::LoadFromWICFile(path, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &md, img);
			if (FAILED(result)) // WIC failed try with HDR
			{
				result = DirectX::LoadFromHDRFile(path, &md, img);
				if (FAILED(result))
				{
					LOG_ERROR("Cannot load the texture.");
				}
			}
		}
	}
	else // DDS success
	{
		result = DirectX::Decompress(img.GetImages(), img.GetImageCount(), md, DXGI_FORMAT_UNKNOWN, dcmprsdImg);

		img = std::move(dcmprsdImg);
	}

	imgResult = &img;
	if (options.flipVertical && options.flipHorizontal)
	{
		result = DirectX::FlipRotate(img.GetImages(),
									 img.GetImageCount(),
									 img.GetMetadata(),
									 DirectX::TEX_FR_FLAGS::TEX_FR_ROTATE180,
									 flippedImg);

		if (!FAILED(result))
		{
			imgResult = &flippedImg;
		}
	}
	else if (options.flipVertical)
	{
		result = DirectX::FlipRotate(img.GetImages(),
									 img.GetImageCount(),
									 img.GetMetadata(),
									 DirectX::TEX_FR_FLAGS::TEX_FR_FLIP_VERTICAL,
									 flippedImg);

		if (!FAILED(result))
		{
			imgResult = &flippedImg;
		}
	}
	else if (options.flipHorizontal)
	{
		result = DirectX::FlipRotate(img.GetImages(),
									 img.GetImageCount(),
									 img.GetMetadata(),
									 DirectX::TEX_FR_FLAGS::TEX_FR_FLIP_HORIZONTAL,
									 flippedImg);

		if (!FAILED(result))
		{
			imgResult = &flippedImg;
		}
	}

	// narrowString = resource->GetLibraryPath() + DDS_TEXTURE_EXTENSION;
	// wideString = std::wstring(narrowString.begin(), narrowString.end());
	// path = wideString.c_str();

	GLint internalFormat;
	GLenum format, type;

	switch (imgResult->GetMetadata().format)
	{
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			internalFormat = GL_RGBA32F;
			format = GL_RGBA;
			type = GL_FLOAT;
			break;
		case DXGI_FORMAT_R8_UNORM:
			internalFormat = GL_R8;
			format = GL_RED;
			type = GL_BYTE;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			internalFormat = GL_RGBA8;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			internalFormat = GL_RGBA8;
			format = GL_BGRA;
			type = GL_UNSIGNED_BYTE;
			break;
		case DXGI_FORMAT_B5G6R5_UNORM:
			internalFormat = GL_RGB8;
			format = GL_BGR;
			type = GL_UNSIGNED_BYTE;
			break;
		case DXGI_FORMAT_BC1_UNORM:
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
			break;
		default:
			assert(false && "Unsupported format");
	}

	resource->SetWidth((unsigned int) imgResult->GetMetadata().width);
	resource->SetHeight((unsigned int) imgResult->GetMetadata().height);

	resource->SetInternalFormat(internalFormat);
	resource->SetFormat(format);
	resource->SetImageType(type);

	resource->SetPixelsSize((unsigned int) imgResult->GetPixelsSize());

	resource->SetPixels(
		std::vector<uint8_t>(imgResult->GetPixels(), imgResult->GetPixels() + imgResult->GetPixelsSize()));

	char* buffer{};
	unsigned int size;
	Save(resource, buffer, size);
	App->GetModule<ModuleFileSystem>()->Save(
		(resource->GetLibraryPath() + GENERAL_BINARY_EXTENSION).c_str(), buffer, size);

	delete buffer;
}

void TextureImporter::Save(const std::shared_ptr<ResourceTexture>& resource, char*& fileBuffer, unsigned int& size)
{
	unsigned int header[6] = { resource->GetWidth(),		  resource->GetHeight(),	resource->GetFormat(),
							   resource->GetInternalFormat(), resource->GetImageType(), resource->GetPixelsSize() };

	unsigned int options[5] = { static_cast<unsigned int>(resource->GetLoadOptions().min),
								static_cast<unsigned int>(resource->GetLoadOptions().mag),
								static_cast<unsigned int>(resource->GetLoadOptions().wrapS),
								static_cast<unsigned int>(resource->GetLoadOptions().wrapT),
								resource->GetLoadOptions().mipMap };

	size = sizeof(header) + sizeof(unsigned char) * resource->GetPixelsSize() + sizeof(options);

	char* cursor = new char[size];

	fileBuffer = cursor;

	unsigned int bytes = sizeof(header);
	memcpy(cursor, header, bytes);

	cursor += bytes;

	bytes = sizeof(unsigned char) * resource->GetPixelsSize();
	memcpy(cursor, &(resource->GetPixels()[0]), bytes);

	cursor += bytes;

	bytes = sizeof(options);
	memcpy(cursor, options, bytes);
}

void TextureImporter::Load(const char* fileBuffer, std::shared_ptr<ResourceTexture> resource)
{
	unsigned int header[6];
	memcpy(header, fileBuffer, sizeof(header));

	resource->SetWidth(header[0]);
	resource->SetHeight(header[1]);
	resource->SetFormat(header[2]);
	resource->SetInternalFormat(header[3]);
	resource->SetImageType(header[4]);
	resource->SetPixelsSize(header[5]);

	fileBuffer += sizeof(header);

	unsigned char* pixelsPointer = new unsigned char[resource->GetPixelsSize()];
	memcpy(pixelsPointer, fileBuffer, sizeof(unsigned char) * resource->GetPixelsSize());
	resource->SetPixels(std::vector<unsigned char>(pixelsPointer, pixelsPointer + resource->GetPixelsSize()));

	fileBuffer += sizeof(unsigned char) * resource->GetPixelsSize();

	delete[] pixelsPointer;

#ifndef ENGINE
	unsigned int options[5];
	memcpy(options, fileBuffer, sizeof(options));

	resource->GetLoadOptions().min = (TextureMinFilter) options[0];
	resource->GetLoadOptions().mag = (TextureMagFilter) options[1];
	resource->GetLoadOptions().wrapS = (TextureWrap) options[2];
	resource->GetLoadOptions().wrapT = (TextureWrap) options[3];
	resource->GetLoadOptions().mipMap = options[4];
#endif // ENGINE
}