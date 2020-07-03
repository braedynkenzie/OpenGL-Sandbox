#pragma once

#include "Test.h"

#include "Shader.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Camera.h"

namespace test
{
	class TestCubemapping : public Test
	{
	private:
		static TestCubemapping* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_Shader;
		Shader* m_SkyboxShader;
		Texture* m_CubeTexture;
		Texture* m_SkyboxTexture;
		VertexArray*  m_VA_Cube;
		VertexBuffer* m_VB_Cube;
		IndexBuffer*  m_IB_Cube;
		VertexArray*  m_VA_Skybox;
		VertexBuffer* m_VB_Skybox;
		IndexBuffer*  m_IB_Skybox;
	public: 
		TestCubemapping(GLFWwindow*& mainWindow);
		~TestCubemapping();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestCubemapping* GetInstance() { return instance; }

	};
}
