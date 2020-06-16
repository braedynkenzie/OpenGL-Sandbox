#include "Texture.h"

#include "vendor/stb_image/stb_image.h"

Texture::Texture(const std::string& filepath)
	: m_RendererID(0), m_Filepath(filepath), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BytesPerPixel(0)
{
	// Load texture from image
	stbi_set_flip_vertically_on_load(true);
	m_LocalBuffer = stbi_load(filepath.c_str(), &m_Width, &m_Height, &m_BytesPerPixel, 4);

	// Generate and bind OpenGL texture
	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

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

Texture::~Texture()
{
	// Delete texture data on GPU
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int textureSlot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + textureSlot));
	GLCall(glBindTexture(GL_TEXTURE_2D, textureSlot));
}

void Texture::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
