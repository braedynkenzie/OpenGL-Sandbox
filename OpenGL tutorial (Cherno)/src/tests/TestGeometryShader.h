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
#include <Model.h>

namespace test
{
	class TestGeometryShader : public Test
	{
	private:
		static TestGeometryShader* instance;
		GLFWwindow* m_MainWindow;
		bool modelLoaded;
		Model* m_BackpackModel;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_Shader;
	public:
		TestGeometryShader(GLFWwindow*& mainWindow);
		~TestGeometryShader();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestGeometryShader* GetInstance() { return instance; }

	};
}
