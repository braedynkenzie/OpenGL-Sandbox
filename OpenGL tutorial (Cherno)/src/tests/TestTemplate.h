#pragma once

#include "Test.h"
#include "Camera.h"

namespace test
{
	class TestTemplate : public Test
	{
	private:
		static TestTemplate* instance;
		GLFWwindow* m_MainWindow;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		Shader* m_Shader;
	public: 
		TestTemplate(GLFWwindow*& mainWindow);
		~TestTemplate();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
		void OnActivated() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestTemplate* GetInstance() { return instance; }

	};
}
