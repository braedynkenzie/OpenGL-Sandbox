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
	class TestInstancedRendering : public Test
	{
	private:
		static TestInstancedRendering* instance;
		GLFWwindow* m_MainWindow;
		bool modelsLoaded;
		Model* m_PlanetModel;
		Model* m_AsteroidModel;
		Texture* m_AsteroidTexture;
		unsigned int m_AsteroidCount;
		glm::mat4* m_AsteroidModelMatrices;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_ModelShader;
		// Skybox data
		Shader* m_SkyboxShader;
		Texture* m_SkyboxTexture;
		VertexArray* m_VA_Skybox;
		VertexBuffer* m_VB_Skybox;
		IndexBuffer* m_IB_Skybox;
	public:
		TestInstancedRendering(GLFWwindow*& mainWindow);
		~TestInstancedRendering();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestInstancedRendering* GetInstance() { return instance; }

	};
}
