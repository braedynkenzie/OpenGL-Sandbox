#include "TestTexture2D.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#include "imgui\imgui.h"

namespace test
{
	TestTexture2D::TestTexture2D()
		: m_ModelTranslation1(glm::vec3(0.0f, 0.0f, 0.0f)), 
		m_ModelTranslation2(glm::vec3(0.5f, 0.0f, -0.5f)),
		m_ModelRotationY(0.0f), m_ModelRotationZ(0.0f), m_ModelScale(1.0f),
		m_SCREEN_WIDTH(800), m_SCREEN_HEIGHT(600)
	{
		// Create vertice positions
		float vertices[] = {
			// positions -- tex coords 
			  -0.5, -0.5,    0.0, 0.0,
			   0.5,  0.5,    1.0, 1.0,
			  -0.5,  0.5,    0.0, 1.0,
			   0.5, -0.5,    1.0, 0.0,
		};

		unsigned int indices[]{
			0, 1, 2,
			3, 0, 1
		};

		m_VA = std::make_unique<VertexArray>();

		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB = std::make_unique<VertexBuffer>(vertices, 4 * 4 * sizeof(float));

		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout layout;
		layout.Push<float>(2);
		layout.Push<float>(2);
		m_VA->AddBuffer(*m_VB, layout);

		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB = std::make_unique<IndexBuffer>(indices, 6);

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");

		// Bind shader program and set uniforms
		m_Shader->Bind();
		m_Texture = std::make_unique<Texture>("res/textures/tree_render_texture.png");
		m_Texture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_Shader->SetUniform1i("u_Texture", 0);

		// Unbind everything
		m_VA->Unbind();
		m_VB->Unbind();
		m_IB->Unbind();
		m_Shader->Unbind();

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestTexture2D::~TestTexture2D()
	{
	}

	void TestTexture2D::OnUpdate(float deltaTime)
	{
	}

	void TestTexture2D::OnRender()
	{
		GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		Renderer renderer;

		// Bind shader and set its per frame uniforms
		m_Shader->Bind();
		m_Shader->SetUniform4f("u_Color", 0.2f, ((sin(glfwGetTime()) + 1.0f) / 2.0f), 0.8f, 1.0f);
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
		glm::mat4 proj = glm::perspective(90.0f, (float)m_SCREEN_WIDTH / (float)m_SCREEN_HEIGHT, 0.1f, 10.0f);
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA, *m_IB, *m_Shader);

		// Change model matrix and render a second square
		modelMatrix = glm::translate(modelMatrix, m_ModelTranslation2);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA, *m_IB, *m_Shader);
	}

	void TestTexture2D::OnImGuiRender()
	{
		ImGui::SliderFloat3("Model 1 translation", &m_ModelTranslation1.x, 0.0f, 10.0f);
		ImGui::SliderFloat3("Model 2 translation", &m_ModelTranslation2.x, 0.0f, 10.0f);
		ImGui::SliderFloat("Model Z axis rotation", &m_ModelRotationZ, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Y axis rotation", &m_ModelRotationY, 0.0f, 360.0f);
		ImGui::SliderFloat("Model scale", &m_ModelScale, -1.0f, 10.0f);
	}
}

