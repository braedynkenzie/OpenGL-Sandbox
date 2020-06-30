#pragma once

#include "Test.h"

#include "FrameBuffer.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Camera.h"

namespace test
{
	class TestManualFramebuffer : public Test
	{
	private:
		static TestManualFramebuffer* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		FrameBuffer* m_FBO;
		unsigned int m_RenderBufferID;
		VertexArray* m_VA_Ground;
		VertexBuffer* m_VB_Ground;
		IndexBuffer* m_IB_Ground;
		VertexArray* m_VA_Quad;
		VertexBuffer* m_VB_Quad;
		IndexBuffer* m_IB_Quad;
		Shader* m_Shader;
		Shader* m_QuadShader;
		Texture* m_WaterTexture;
		unsigned int m_FramebufferTexture;

	public: 
		TestManualFramebuffer(GLFWwindow*& mainWindow);
		~TestManualFramebuffer();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestManualFramebuffer* GetInstance() { return instance; }
	};
}
