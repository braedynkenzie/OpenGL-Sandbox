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
	class TestShadowMapping : public Test
	{
	private:
		static TestShadowMapping* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		Camera m_Camera;
		Shader* m_Shader;
		Shader* m_ShadowDepthMapShader;
		Texture* m_CubeTexture;
		Texture* m_GroundTexture;
		VertexArray* m_VA_Cube;
		VertexBuffer* m_VB_Cube;
		IndexBuffer* m_IB_Cube;
		VertexArray* m_VA_Ground;
		VertexBuffer* m_VB_Ground;
		IndexBuffer* m_IB_Ground;
		// Flashlight properties
		bool m_IsFlashlightOn;
		glm::vec3 m_FlashlightColour;
		glm::vec3 m_FlashlightDiffuseIntensity;
		glm::vec3 m_FlashlightAmbientIntensity;
		glm::vec3 m_FlashlightSpecularIntensity;
		glm::vec3 m_FlashlightDiffuseColour;
		glm::vec3 m_FlashlightAmbientColour;
		// Directional light properties
		glm::vec3 m_DirLightDirection;
		glm::vec3 m_DirLightAmbient;
		glm::vec3 m_DirLightDiffuse;
		glm::vec3 m_DirLightSpecular;
		// Shadow map properties
		unsigned int m_ShadowMapWidth;
		unsigned int m_ShadowMapHeight;
		unsigned int m_ShadowDepthMap;
		unsigned int m_DepthMapFBO;
	public:
		TestShadowMapping(GLFWwindow*& mainWindow);
		~TestShadowMapping();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		void ShadowResolution(int dir);

		Camera* GetCamera() { return &m_Camera; }
		static TestShadowMapping* GetInstance() { return instance; }

	};
}
