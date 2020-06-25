#pragma once

#include "Test.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Camera.h"

#include <memory>


namespace test
{
	class TestPhongLighting : public Test
	{
	private:
		static TestPhongLighting* instance;
		GLFWwindow* m_MainWindow;
		std::unique_ptr<VertexArray> m_VA_Ground;
		std::unique_ptr<VertexBuffer> m_VB_Ground;
		std::unique_ptr<IndexBuffer> m_IB_Ground;
		std::unique_ptr<VertexArray> m_VA_PointLight;
		std::unique_ptr<VertexBuffer> m_VB_PointLight;
		std::unique_ptr<IndexBuffer> m_IB_PointLight;
		std::unique_ptr<Shader> m_GroundShader;
		std::unique_ptr<Shader> m_PointLightsShader;
		std::unique_ptr<Texture> m_Texture;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		// Flashlight properties
		bool m_IsFlashlightOn;
		glm::vec3 m_FlashlightColour;
		glm::vec3 m_fl_diffuseIntensity;
		glm::vec3 m_fl_ambientIntensity;
		glm::vec3 m_fl_specularIntensity;
		glm::vec3 m_fl_diffuseColour;
		glm::vec3 m_fl_ambientColour;
		// Point light properties
		glm::vec3 m_PointLightPos;
		glm::vec3 m_pl_diffuseIntensity;
		glm::vec3 m_pl_ambientIntensity;
		glm::vec3 m_pl_specularIntensity;

	public:

		TestPhongLighting(GLFWwindow*& mainWindow);
		~TestPhongLighting();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestPhongLighting* GetInstance() { return instance; }
	};
}
