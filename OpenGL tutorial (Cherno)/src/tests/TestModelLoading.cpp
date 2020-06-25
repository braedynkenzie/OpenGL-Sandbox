#include "TestModelLoading.h"

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <vendor\stb_image\stb_image.h>
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackModelTest(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackModelTest(GLFWwindow* window, double xOffset, double yOffset);
	void processInputModelTest(GLFWwindow* window);

	// Init static variable
	TestModelLoading* TestModelLoading::instance;

	TestModelLoading::TestModelLoading(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		modelLoaded(false),
		m_BackpackPos(glm::vec3(0.0f, 0.0f, 0.0f)),
		m_BackpackModel(nullptr),
		m_Shader(new Shader("res/shaders/Backpack.shader")),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 4.0f)),
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(Camera(m_CameraPos, 60.0f)),
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_fl_diffuseIntensity(glm::vec3(1.0f)),
		m_fl_ambientIntensity(glm::vec3(0.4f)), m_fl_specularIntensity(glm::vec3(0.2f)),
		m_fl_diffuseColour( m_FlashlightColour * m_fl_diffuseIntensity), 
		m_fl_ambientColour(m_fl_diffuseColour * m_fl_ambientIntensity)
	{
		instance = this;

		// Mouse cursor should start in the middle of the window
		lastCursorX = SCREEN_WIDTH / 2.0f;
		lastCursorY = SCREEN_HEIGHT / 2.0f;

		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackModelTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackModelTest);

		// Create vertice positions
		float vertices[] = {
		 //       positions         --     normals    --    tex coords    
			  -800.0, -10.0, -800.0,    0.0, 1.0, 0.0,	    0.0, 100.0,
			   800.0, -10.0,  800.0,    0.0, 1.0, 0.0,	  100.0,   0.0,
			  -800.0, -10.0,  800.0,    0.0, 1.0, 0.0,	    0.0,   0.0,
			   800.0, -10.0, -800.0,    0.0, 1.0, 0.0,	  100.0, 100.0,
		};

		unsigned int indices[]{
			0, 2, 1,
			3, 0, 1,
		};

		m_VA = std::make_unique<VertexArray>();

		// Init Vertex Buffer and bind to Vertex Array (m_VA)
		m_VB = std::make_unique<VertexBuffer>(vertices, 8 * 4 * sizeof(float));

		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout layout;
		layout.Push<float>(3); // Vertex position,vec3
		layout.Push<float>(3); // Normals, vec3
		layout.Push<float>(2); // Texture coordinates, vec2
		m_VA->AddBuffer(*m_VB, layout);

		// Init index buffer and bind to Vertex Array (m_VA)
		m_IB = std::make_unique<IndexBuffer>(indices, 6);

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

	TestModelLoading::~TestModelLoading()
	{
	}

	void TestModelLoading::OnUpdate(float deltaTime)
	{
	}

	void TestModelLoading::OnRender()
	{
		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process WASD keyboard camera movement
		processInputModelTest(m_MainWindow);

		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 2.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor, 
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		Renderer renderer;

		// Bind shader and set any 'per frame' uniforms
		m_Shader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, 0.0, 0.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_Shader->SetMatrix4f("model", modelMatrix);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);

		// Update camera's viewing position each frame
		m_Shader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);

		// Flashlight's properties
		//
		m_Shader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_Shader->SetVec3("u_Flashlight.ambient", m_fl_ambientColour);
		m_Shader->SetVec3("u_Flashlight.diffuse", m_fl_diffuseColour);
		m_Shader->SetVec3("u_Flashlight.specular", m_fl_specularIntensity);
		// Flashlight attenuation properties
		m_Shader->SetFloat("u_Flashlight.constant", 1.0f);
		m_Shader->SetFloat("u_Flashlight.linear", 0.06f);
		m_Shader->SetFloat("u_Flashlight.quadratic", 0.005f);
		// Flashlight position and direction
		m_Shader->SetVec3f("u_Flashlight.position", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
		m_Shader->SetVec3f("u_Flashlight.direction", m_Camera.Front.x, m_Camera.Front.y, m_Camera.Front.z);
		// Flashlight cutoff angle
		m_Shader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(5.0f)));
		m_Shader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(30.0f)));
		// Bind ground texture
		m_GroundTexture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_Shader->SetUniform1i("texture_diffuse0", 0);
		// Render the ground
		renderer.Draw(*m_VA, *m_IB, *m_Shader);

		// Load model's uniforms and render the loaded backpack model
		m_BackpackModel->Draw(m_Shader);
	}

	void TestModelLoading::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestModelLoading::OnActivated()
	{
		// Only want to load the model the first time we activate this OpenGL test
		if (!modelLoaded)
		{
			// Flip texture along y axis before loading
			stbi_set_flip_vertically_on_load(true);
			m_BackpackModel = new Model((char*)"res/models/backpack/backpack.obj");
			//m_BackpackModel = new Model((char*)"res/models/nature/BlenderNatureAsset.obj");
			modelLoaded = true;
		}

		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Enable face culling
		glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT); // cull front faces
		glCullFace(GL_BACK); // cull back faces
		glFrontFace(GL_CCW); // tell OpenGL that front faces have CCW winding order

		// Load ground texture
		m_GroundTexture = std::make_unique<Texture>("res/textures/dirt_ground_texture.png");

		// Bind shader program and set uniforms
		m_Shader->Bind();
		m_Shader->SetVec3f("u_Material.specular", 0.5f, 0.5f, 0.5f);
		m_Shader->SetFloat("u_Material.shininess", 16.0f);
		// Reset MVP matrices on activation
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_Shader->SetMatrix4f("model", modelMatrix);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);
		// Set texture mode to repeat
		// TODO mipmapping
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputModelTest(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackModelTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackModelTest);
	}

	void scroll_callbackModelTest(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestModelLoading* modelTest = test::TestModelLoading::GetInstance();
		Camera* phongCamera = modelTest->GetCamera();
		phongCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackModelTest(GLFWwindow* window, double xpos, double ypos)
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

		test::TestModelLoading* modelTest = test::TestModelLoading::GetInstance();
		Camera* phongCamera = modelTest->GetCamera();
		phongCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void processInputModelTest(GLFWwindow* window) {
		test::TestModelLoading* modelTest = test::TestModelLoading::GetInstance();
		Camera* phongCamera = modelTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(FORWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(BACKWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(LEFT, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			phongCamera->ProcessKeyboardForWalkingView(RIGHT, deltaTime, -0.2f);
	}
}
