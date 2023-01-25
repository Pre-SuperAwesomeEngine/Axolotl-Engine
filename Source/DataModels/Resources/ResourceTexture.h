#pragma once

#include "Resource.h"
#include <memory>

struct LoadOptionsTexture;

struct ImportOptionsTexture
{
	bool flip;

	ImportOptionsTexture() :
		flip(true)
	{}
};

class ResourceTexture : public Resource
{
public:
	ResourceTexture(UID resourceUID, const std::string& fileName, const std::string& assetsPath, const std::string& libraryPath);
	~ResourceTexture() override;

	ResourceType GetType() const override;

	void SaveOptions(Json& meta) override;
	void LoadOptions(Json& meta) override;

	unsigned int GetGlTexture() const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetFormat() const;
	unsigned int GetInternalFormat() const;
	unsigned int GetImageType() const;
	const std::vector<uint8_t>& GetPixels() const;
	unsigned int GetPixelsSize() const;

	std::shared_ptr<ImportOptionsTexture>& GetImportOptions();
	std::shared_ptr<LoadOptionsTexture>& GetLoadOptions();

	void SetWidth(unsigned int width);
	void SetHeight(unsigned int height);
	void SetFormat(unsigned int format);
	void SetInternalFormat(unsigned int internalFormat);
	void SetImageType(unsigned int imageType);
	void SetPixels(std::vector<uint8_t>& pixels);
	void SetPixelsSize(unsigned int pixelsSize);

protected:
	void InternalLoad() override;
	void InternalUnload() override;
private:
	void CreateTexture();

	unsigned int glTexture = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int format = 0;
	unsigned int internalFormat = 0;
	unsigned int imageType = 0;
	std::vector<uint8_t> pixels;
	unsigned int pixelsSize = 0;

	std::shared_ptr<LoadOptionsTexture> loadOptions;
	std::shared_ptr<ImportOptionsTexture> importOptions;
};

inline ResourceTexture::~ResourceTexture()
{
	this->Unload();
}

inline ResourceType ResourceTexture::GetType() const
{
	return ResourceType::Texture;
}

inline unsigned int ResourceTexture::GetGlTexture() const
{
	return glTexture;
}

inline unsigned int ResourceTexture::GetWidth() const
{
	return width;
}

inline unsigned int ResourceTexture::GetHeight() const
{
	return height;
}

inline unsigned int ResourceTexture::GetFormat() const
{
	return format;
}

inline unsigned int ResourceTexture::GetInternalFormat() const
{
	return internalFormat;
}

inline unsigned int ResourceTexture::GetImageType() const
{
	return imageType;
}

inline const std::vector<uint8_t>& ResourceTexture::GetPixels() const
{
	return pixels;
}

inline unsigned int ResourceTexture::GetPixelsSize() const
{
	return pixelsSize;
}

inline std::shared_ptr<ImportOptionsTexture>& ResourceTexture::GetImportOptions()
{
	return this->importOptions;
}

inline std::shared_ptr<LoadOptionsTexture>& ResourceTexture::GetLoadOptions()
{
	return this->loadOptions;
}

inline void ResourceTexture::SetWidth(unsigned int width)
{
	this->width = width;
}

inline void ResourceTexture::SetHeight(unsigned int height)
{
	this->height = height;
}

inline void ResourceTexture::SetFormat(unsigned int format)
{
	this->format = format;
}

inline void ResourceTexture::SetInternalFormat(unsigned int internalFormat)
{
	this->internalFormat = internalFormat;
}

inline void ResourceTexture::SetImageType(unsigned int imageType)
{
	this->imageType = imageType;
}

inline void ResourceTexture::SetPixels(std::vector<uint8_t>& pixels)
{
	this->pixels = pixels;
}

inline void ResourceTexture::SetPixelsSize(unsigned int pixelsSize)
{
	this->pixelsSize = pixelsSize;
}
