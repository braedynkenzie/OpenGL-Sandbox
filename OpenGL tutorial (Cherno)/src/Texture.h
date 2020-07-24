#pragma once

#include "Renderer.h"

class Texture
{

private:
	unsigned int m_RendererID;
	std::string m_Filepath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BytesPerPixel;

public:
	Texture(const std::string& filepath, const bool requiresGammaCorrection = true, const bool flipOnLoad = true);
	Texture(const std::vector<std::string>& cubemapFilepaths, bool flipOnLoad = false);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	void BindCubemap(unsigned int slot = 0) const;
	void UnbindCubemap() const;

	inline int GetWidth() const { return m_Width; }
	inline int GetHeight() const { return m_Height; }
	inline unsigned int GetID() const { return m_RendererID; }

	void BindAndSetRepeating(unsigned int textureSlot) const {
		Bind(textureSlot);
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	unsigned int LoadCubemap(std::vector<std::string> texturePaths);

};