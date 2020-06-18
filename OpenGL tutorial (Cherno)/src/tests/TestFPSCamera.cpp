#include "TestFPSCamera.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"


namespace test
{
	// Function declarations
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);

	TestFPSCamera* TestFPSCamera::instance;

	float lastCursorX;
	float lastCursorY;
	float yaw = 0.0f;
	float pitch = 0.0f;
	bool firstMouseCapture = true;

	TestFPSCamera::TestFPSCamera(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow), 
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)), 
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), 
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)), 
		m_Camera(Camera(m_CameraPos)),
		m_FirstMouseCapture(true),
		m_SCREEN_WIDTH(800), m_SCREEN_HEIGHT(600),
		m_LastCursorX((float) m_SCREEN_WIDTH / 2.0f),
		m_LastCursorY((float) m_SCREEN_HEIGHT / 2.0f)
	{
		instance = this;

		float lastCursorX = m_SCREEN_WIDTH / 2.0f;
		float lastCursorY = m_SCREEN_HEIGHT / 2.0f;

		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callback); // todo callback
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callback);

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

	TestFPSCamera::~TestFPSCamera()
	{
	}

	void TestFPSCamera::OnUpdate(float deltaTime)
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void TestFPSCamera::OnRender()
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
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		// View/Projection transformations
		glm::mat4 proj = glm::perspective(glm::radians(m_Camera.Zoom), (float)m_SCREEN_WIDTH / (float)m_SCREEN_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = m_Camera.GetViewMatrix();
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA, *m_IB, *m_Shader);
	}

	void TestFPSCamera::OnImGuiRender()
	{
		// ImGui interface
	}

	void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestFPSCamera* cameraTest = test::TestFPSCamera::GetInstance();
		Camera* camera = cameraTest->GetCamera();
		camera->ProcessMouseScroll(yOffset);
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos)
	{
		// Fixes first mouse cursor capture by OpenGL window
		if (firstMouseCapture)
		{
			lastCursorX = xpos;
			lastCursorY = ypos;
			firstMouseCapture = false;
		}
		float xOffset = xpos - lastCursorX;
		float yOffset = lastCursorY - ypos; // reverse the y-coordinates
		float cursorSensitivity = 0.08f;
		xOffset *= cursorSensitivity;
		yOffset *= cursorSensitivity;
		yaw += xOffset;
		pitch += yOffset;
		lastCursorX = xpos;
		lastCursorY = ypos;

		test::TestFPSCamera* test = test::TestFPSCamera::GetInstance();
		Camera* camera = test->GetCamera();
		camera->ProcessMouseMovement(xOffset, yOffset);
	}
}
