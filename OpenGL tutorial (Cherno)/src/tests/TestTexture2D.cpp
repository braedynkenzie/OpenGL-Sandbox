#include "TestTexture2D.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <tests\TestClearColour.h>
#include <string>	

namespace test
{
	const int TEXTURE_COUNT = 2;

	TestTexture2D::TestTexture2D(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow), m_ActiveTexture(0),
		m_ModelTranslation(glm::vec3(0.0f, 0.0f, 0.0f)),
		m_ModelRotationX(0.0), m_ModelRotationY(0.0f), m_ModelRotationZ(0.0f), 
		m_ModelScale(1.0f), m_SCREEN_WIDTH(800), m_SCREEN_HEIGHT(600)
	{
		// Create vertice positions
		float vertices[] = {
			// positions      --  tex coords 
			  -0.5, -0.5, -0.5,    0.0, 0.0, // Cube front
			   0.5,  0.5, -0.5,    1.0, 1.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0, // Cube back
			   0.5,  0.5,  0.5,    1.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,

			  -0.5, -0.5, -0.5,    0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,

			   0.5, -0.5,  0.5,    0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,

			  -0.5,  0.5, -0.5,    1.0, 1.0, // Cube top
			   0.5,  0.5,  0.5,    0.0, 0.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,
		};

		unsigned int indices[]{
			0, 1, 2,
			3, 0, 1,

			4, 5, 6,
			7, 4, 5,

			8, 9, 10,
			11, 8, 9,

			12, 13, 14,
			15, 12, 13,

			16, 17, 18,
			19, 16, 17,

			20, 21, 22,
			23, 20, 21
		};

		m_VA = std::make_unique<VertexArray>();

		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB = std::make_unique<VertexBuffer>(vertices, 5 * 4 * 6 * sizeof(float));

		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);
		m_VA->AddBuffer(*m_VB, layout);

		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB = std::make_unique<IndexBuffer>(indices, 6 * 6);

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");

		// Load/create textures
		m_Textures[0] = std::make_unique<Texture>("res/textures/tree_render_texture.png");
		m_Textures[1] = std::make_unique<Texture>("res/textures/tree_render_texture2.png");
		
		// Bind shader program and set uniforms
		m_Shader->Bind();
		for (int i = 0; i < TEXTURE_COUNT; i++)
		{
			m_Textures[i]->Bind(i); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
			std::string textureName = "u_Texture" + std::to_string(i);
			m_Shader->SetUniform1i(textureName, i);
		}

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
		float* clearColour = test::TestClearColour::GetClearColour();
		GLCall(glClearColor(clearColour[0], clearColour[1], clearColour[2], clearColour[3]));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		Renderer renderer;

		// Bind shader and set its per frame uniforms
		m_Shader->Bind();
		m_Shader->SetUniform4f("u_Color", 0.2f, ((sin(glfwGetTime()) + 1.0f) / 2.0f), 0.8f, 1.0f);
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, m_ModelTranslation);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(m_ModelRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(m_ModelRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::rotate(modelMatrix, glm::radians(m_ModelRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f + m_ModelScale));
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -5.0f));
		// glm::mat4 proj = glm::ortho(0.0f, (float)SCREEN_WIDTH / 100, 0.0f, (float)SCREEN_HEIGHT / 100, -10.0f, 10.0f);
		glm::mat4 proj = glm::perspective(glm::radians(55.0f), (float)m_SCREEN_WIDTH / (float)m_SCREEN_HEIGHT, 0.1f, 10.0f);
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA, *m_IB, *m_Shader);
	}

	void TestTexture2D::OnImGuiRender()
	{
		// ImGui interface
		ImGui::SliderFloat3("Model 1 translation", &m_ModelTranslation.x, -5.0f, 5.0f);
		ImGui::SliderFloat("Model X axis rotation", &m_ModelRotationX, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Y axis rotation", &m_ModelRotationY, 0.0f, 360.0f);
		ImGui::SliderFloat("Model Z axis rotation", &m_ModelRotationZ, 0.0f, 360.0f);
		ImGui::SliderFloat("Model scale", &m_ModelScale, -1.0f, 5.0f);
		if (ImGui::Button("Switch texture"))
		{
			if (m_ActiveTexture == 0)
			{
				m_Shader->SetUniform1i("u_ActiveTexture", 1);
				m_ActiveTexture = 1;
			}
			else if (m_ActiveTexture == 1)
			{
				m_Shader->SetUniform1i("u_ActiveTexture", 0);
				m_ActiveTexture = 0;
			}
		}
			
	}

	void TestTexture2D::OnActivated()
	{
		// Bind shader program and reset any uniforms
		m_Shader->Bind();
		for (int i = 0; i < TEXTURE_COUNT; i++)
		{
			m_Textures[i]->Bind(i); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
			std::string textureName = "u_Texture" + std::to_string(i);
			m_Shader->SetUniform1i(textureName, i);
		}
	}
}

