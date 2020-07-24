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
	class TestSSAO : public Test
	{
	private:
		static TestSSAO* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_Shader;
		Texture* m_CubeTexture;
		VertexArray* m_VA_Cube;
		VertexBuffer* m_VB_Cube;
		IndexBuffer* m_IB_Cube;
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
