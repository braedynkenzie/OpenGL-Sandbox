#include "Texture.h"

#include "vendor/stb_image/stb_image.h"
#include <iostream>

Texture::Texture(const std::string& filepath)
	: m_RendererID(0), m_Filepath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BytesPerPixel(0)
{
	// Load texture from image
	stbi_set_flip_vertically_on_load(true);
	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 4);

	// Generate and bind OpenGL texture
	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set OpenGL texture parameters
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	// Send the local image buffer to the GPU
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	// Delete local texture buffer now that we've sent it to the GPU
	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);
}

// Load cubemap texture
Texture::Texture(const std::vector<std::string>& cubemapFilepaths, bool flipOnLoad)
	: m_RendererID(0), 
	m_Filepath(cubemapFilepaths[0]), // TODO, currently just stores the first filepath 
	m_LocalBuffer(nullptr), 
	m_Width(0), m_Height(0), m_BytesPerPixel(0)
{
	stbi_set_flip_vertically_on_load(flipOnLoad);

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));

	for (unsigned int i = 0; i < cubemapFilepaths.size(); i++)
	{
		unsigned char* data = stbi_load(cubemapFilepaths[i].c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 0);

		if (data)
		{
			GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
		}
		else
		{
			std::cout << "[ERROR] Cubemap texture failed to load at path: " << cubemapFilepaths[i] << std::endl;
		}

		stbi_image_free(data);
	}
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
}

Texture::~Texture()
{
	// Delete texture data on GPU
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int textureSlot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + textureSlot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture::BindCubemap(unsigned int textureSlot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + textureSlot));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID));
}

void Texture::UnbindCubemap() const
{
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}
