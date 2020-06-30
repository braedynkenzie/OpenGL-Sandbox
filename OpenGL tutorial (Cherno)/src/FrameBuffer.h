#pragma once

class FrameBuffer
{
private:
	unsigned int m_RendererID;

public:
	FrameBuffer();
	~FrameBuffer();

	void Bind() const;
	void Unbind() const;
};