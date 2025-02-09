#pragma once

#include "Resource.h"

enum class TextureCompression
{
	NONE,
	DXT1,
	DXT3,
	DXT5,
	BC7
};

enum class TextureMinFilter
{
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR
};

enum class TextureMagFilter
{
	NEAREST,
	LINEAR
};

enum class TextureWrap
{
	REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_REPEAT,
	MIRROR_CLAMP_TO_EDGE
};

struct LoadOptionsTexture
{
	TextureMinFilter min;
	TextureMagFilter mag;
	TextureWrap wrapS;
	TextureWrap wrapT;
	bool mipMap;

	LoadOptionsTexture() :
		min(TextureMinFilter::LINEAR_MIPMAP_LINEAR),
		mag(TextureMagFilter::LINEAR),
		wrapS(TextureWrap::REPEAT),
		wrapT(TextureWrap::REPEAT),
		mipMap(true)
	{
	}
};

struct ImportOptionsTexture
{
	bool flipVertical;
	bool flipHorizontal;

	ImportOptionsTexture() : flipVertical(true), flipHorizontal(false)
	{
	}
};

class ResourceTexture : virtual public Resource
{
public:
	ResourceTexture(UID resourceUID,
					const std::string& fileName,
					const std::string& assetsPath,
					const std::string& libraryPath);
	virtual ~ResourceTexture() override;

	ResourceType GetType() const override;

	void SaveImporterOptions(Json& meta) override;
	void LoadImporterOptions(Json& meta) override;

	void SaveLoadOptions(Json& meta) override;
	void LoadLoadOptions(Json& meta) override;

	unsigned int GetGlTexture() const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	unsigned int GetFormat() const;
	unsigned int GetInternalFormat() const;
	unsigned int GetImageType() const;
	const std::vector<uint8_t>& GetPixels() const;
	unsigned int GetPixelsSize() const;
	const uint64_t& GetHandle();

	ImportOptionsTexture& GetImportOptions();
	LoadOptionsTexture& GetLoadOptions();

	void SetWidth(unsigned int width);
	void SetHeight(unsigned int height);
	void SetFormat(unsigned int format);
	void SetInternalFormat(unsigned int internalFormat);
	void SetImageType(unsigned int imageType);
	void SetPixels(const std::vector<uint8_t>& pixels);
	void SetPixelsSize(unsigned int pixelsSize);

protected:
	void InternalLoad() override;
	void InternalUnload() override;

private:
	void CreateTexture();

	int GetMagFilterEquivalence(TextureMagFilter filter);

	int GetMinFilterEquivalence(TextureMinFilter filter);

	int GetWrapFilterEquivalence(TextureWrap filter);

	unsigned int glTexture = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int format = 0;
	unsigned int internalFormat = 0;
	unsigned int imageType = 0;
	std::vector<uint8_t> pixels;
	unsigned int pixelsSize;

	uint64_t handle;

	LoadOptionsTexture loadOptions;
	ImportOptionsTexture importOptions;
};

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

inline ImportOptionsTexture& ResourceTexture::GetImportOptions()
{
	return this->importOptions;
}

inline LoadOptionsTexture& ResourceTexture::GetLoadOptions()
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

inline void ResourceTexture::SetPixels(const std::vector<uint8_t>& pixels)
{
	this->pixels = pixels;
}

inline void ResourceTexture::SetPixelsSize(unsigned int pixelsSize)
{
	this->pixelsSize = pixelsSize;
}

inline void ResourceTexture::InternalLoad()
{
	CreateTexture();
}
