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
	class TestHDRBloom : public Test
	{
	private:
		static TestHDRBloom* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		FrameBuffer* m_ManualFramebuffer;
		unsigned int m_RenderBufferID;
		VertexArray* m_VA_Ground;
		VertexBuffer* m_VB_Ground;
		IndexBuffer* m_IB_Ground;
		VertexArray* m_VA_Quad;
		VertexBuffer* m_VB_Quad;
		IndexBuffer* m_IB_Quad;
		VertexArray* m_VA_Cube;
		VertexBuffer* m_VB_Cube;
		IndexBuffer* m_IB_Cube;
		Shader* m_HDRLightingShader;
		Shader* m_PointlightsShader;
		Shader* m_QuadShader;
		Texture* m_GroundTexture;
		Texture* m_CubeTexture;
		unsigned int m_HDRBuffer;
		glm::vec3 m_PointLightPositions[2];
		glm::vec3 m_PointLightColours[2];
		float m_LightIntensity;
		float m_LightExposure;
		bool m_UsingHDR;
		// Skybox data
		Shader* m_SkyboxShader;
		Texture* m_SkyboxTexture;
		VertexArray* m_VA_Skybox;
		VertexBuffer* m_VB_Skybox;
		IndexBuffer* m_IB_Skybox;

	public: 
		TestHDRBloom(GLFWwindow*& mainWindow);
		~TestHDRBloom();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		void LightIntensity(const int dir);
		void ExposureLevel(const int dir);
		void ToggleHDR(bool flag);

		Camera* GetCamera() { return &m_Camera; }
		static TestHDRBloom* GetInstance() { return instance; }
	};
}
