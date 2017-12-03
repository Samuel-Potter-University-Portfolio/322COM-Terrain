#include "Texture.h"
#include "Logger.h"

#include <GL\glew.h>
#include <FreeImage.h>


Texture::Texture()
{
	glGenTextures(1, &m_id);
}

Texture::~Texture()
{
	if (m_id != 0)
		glDeleteTextures(1, &m_id);
}

bool Texture::LoadFromFile(const string& file)
{
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	FIBITMAP* data = nullptr;
	bIsLoaded = false;


	// Attempt to work out format from signature
	format = FreeImage_GetFileType(file.c_str(), 0);

	// Attempt to get format from extension
	if (format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(file.c_str());

	// Unable to work out file format
	if (format == FIF_UNKNOWN)
	{
		LOG_ERROR("Unable to deduce format for file '%s'", file.c_str());
		return false;
	}

	if(!FreeImage_FIFSupportsReading(format)) 
	{
		LOG_ERROR("Unable to read file '%s' (Unsupported file format)", file.c_str());
		return false;
	}
	
	data = FreeImage_Load(format, file.c_str());
	if(data == nullptr)
	{
		LOG_ERROR("Unable to read file '%s' (Is the file readable?)", file.c_str());
		return false;
	}


	// Read data
	m_width = FreeImage_GetWidth(data);
	m_height = FreeImage_GetHeight(data);
	uint32 bpp = FreeImage_GetBPP(data); // Bits per Pixel

	if(m_width == 0 || m_height == 0 || bpp == 0)
	{
		LOG_ERROR("Unable to read file '%s' (Read invalid attribs)", file.c_str());
		return false;
	}

	// Convert to 32 bit
	if (bpp != 32)
	{
		FIBITMAP* oldData = data;
		data = FreeImage_ConvertTo32Bits(data);
		FreeImage_Unload(oldData);
	}


	// Load into OpenGL
	BYTE* rawBytes = FreeImage_GetBits(data);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bUsesMipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, rawBytes);
	if(bUsesMipmaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	FreeImage_Unload(data);
	bIsLoaded = true;
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void Texture::SetRepeated(const bool& value) 
{
	bIsRepeated = value;
	if (!bIsLoaded)
		return;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bIsRepeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetSmooth(const bool& value) 
{
	bIsSmooth = value;
	if (!bIsLoaded)
		return;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bIsSmooth ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}