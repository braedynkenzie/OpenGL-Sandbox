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
	class TestFPSCamera : public Test
	{
	private:
		static TestFPSCamera* instance;
		GLFWwindow* m_MainWindow; // todo make unique pointer? prob not
		std::unique_ptr<VertexArray> m_VA;
		std::unique_ptr<VertexBuffer> m_VB;
		std::unique_ptr<IndexBuffer> m_IB;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture;
		glm::vec3 m_CameraPos;
		glm::vec3 m_CameraFront;
		glm::vec3 m_CameraUp;
		Camera m_Camera;
		bool m_FirstMouseCapture;
		const int m_SCREEN_WIDTH, m_SCREEN_HEIGHT;
		float m_LastCursorX;
		float m_LastCursorY;
		float m_Yaw;
		float m_Pitch;

	public:

		TestFPSCamera(GLFWwindow*& mainWindow);
		~TestFPSCamera();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;

		Camera* GetCamera() { return &m_Camera; }
		static TestFPSCamera* GetInstance() { return instance; }
	};
}
