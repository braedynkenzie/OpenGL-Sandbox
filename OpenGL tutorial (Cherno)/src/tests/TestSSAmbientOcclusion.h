#pragma once

#include "Test.h"

#include "Shader.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "FrameBuffer.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"

namespace test
{
	class TestSSAO : public Test
	{
	private:
		static TestSSAO* instance;
		GLFWwindow* m_MainWindow;
		bool modelLoaded;
		Model* m_Model;
		VertexArray* m_VA_Ground;
		VertexBuffer* m_VB_Ground;
		IndexBuffer* m_IB_Ground;
		VertexArray* m_VA_Quad;
		VertexBuffer* m_VB_Quad;
		IndexBuffer* m_IB_Quad;
		Shader* m_GBufferShader;
		Shader* m_QuadShader;
		Texture* m_GroundTexture;
		Texture* m_SecondaryTexture;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		// Deferred Rendering variables
		FrameBuffer* m_GBufferSSAO;
		unsigned int m_RenderBufferID;
		unsigned int m_PositionGBuffer;
		unsigned int m_NormalGBuffer;
		unsigned int m_AlbedoSpecGBuffer;
		// Screen-space Ambient Occlusion variables
		unsigned int m_MaxSamples;
		unsigned int m_NoiseTexture;

	public:
		TestSSAO(GLFWwindow*& mainWindow);
		~TestSSAO();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestSSAO* GetInstance() { return instance; }

	};
}
