#pragma once

#include "Test.h"
#include "Camera.h"
#include "Model.h"

namespace test
{
	class TestModelLoading : public Test
	{
	private:
		static TestModelLoading* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_BackpackPos;
		Model* m_BackpackModel;
		Shader* m_BackpackShader;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera* m_Camera;
		bool m_IsFlashlightOn;
		glm::vec3 m_FlashlightColour;
		glm::vec3 m_fl_diffuseIntensity;
		glm::vec3 m_fl_ambientIntensity;
		glm::vec3 m_fl_specularIntensity;
		glm::vec3 m_fl_diffuseColour;
		glm::vec3 m_fl_ambientColour;

	public: 
		TestModelLoading(GLFWwindow*& mainWindow);
		~TestModelLoading();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return m_Camera; }
		static TestModelLoading* GetInstance() { return instance; }

	};
}
