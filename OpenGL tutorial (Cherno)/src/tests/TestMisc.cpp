#include "TestMisc.h"
#include <GL\glew.h>
#include "Renderer.h"
#include "imgui\imgui.h"
#include <GLFW\glfw3.h>

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

namespace test
{
	TestMisc::TestMisc()
		  : m_ModelTranslation1(glm::vec3(0.0f, 0.0f, 0.0f)), 
			m_ModelTranslation2(glm::vec3(0.5f, 0.0f, -0.5f)),
			m_ModelRotationZ(0.0f), m_ModelRotationY(0.0f), m_ModelScale(1.0f)
	{
	}

	TestMisc::~TestMisc()
	{
	}

	void TestMisc::OnUpdate(float deltaTime)
	{
	}

	void TestMisc::OnRender()
	{
		// Bind shader and set its uniforms
		shader.Bind();
		shader.SetUniform4f("u_Color", 0.2f, ((sin(glfwGetTime()) + 1.0f) / 2.0f), 0.8f, 1.0f);
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, m_ModelTranslation1);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(m_ModelRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(m_ModelRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(m_ModelScale));
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -5.0f));
		// glm::mat4 proj = glm::ortho(0.0f, (float)SCREEN_WIDTH / 100, 0.0f, (float)SCREEN_HEIGHT / 100, -10.0f, 10.0f);
		glm::mat4 proj = glm::perspective(90.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 10.0f);
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		shader.SetMatrix4f("u_MVP", MVP_matrix);
		m_Renderer.Draw(VA, IB, shader);

		// Change model matrix and render a second square
		modelMatrix = glm::translate(modelMatrix, m_ModelTranslation2);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		MVP_matrix = proj * view * modelMatrix;
		shader.SetMatrix4f("u_MVP", MVP_matrix);
		m_Renderer.Draw(VA, IB, shader);
	}

	void TestMisc::OnImGuiRender()
	{
		ImGui::SliderFloat3("Model 1 translation", &m_ModelTranslation1.x, 0.0f, 10.0f);
		ImGui::SliderFloat3("Model 2 translation", &m_ModelTranslation2.x, 0.0f, 10.0f);
		ImGui::SliderFloat("Model Z axis rotation", &m_ModelRotationZ, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Y axis rotation", &m_ModelRotationY, 0.0f, 360.0f);
		ImGui::SliderFloat("Model scale", &m_ModelScale, -1.0f, 10.0f);
	}
}

