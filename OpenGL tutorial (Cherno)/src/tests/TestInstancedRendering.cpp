#include "TestInstancedRendering.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackInstancedRendering(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackInstancedRendering(GLFWwindow* window, double xOffset, double yOffset);
	void processInputInstancedRendering(GLFWwindow* window);
	void mouse_button_callbackInstancedRendering(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestInstancedRendering* TestInstancedRendering::instance;

	TestInstancedRendering::TestInstancedRendering(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		modelsLoaded(false),
		m_PlanetModel(nullptr),
		m_AsteroidModel(nullptr),
		m_AsteroidTexture(new Texture("res/models/rock/rock.png")),
		m_AsteroidCount(50000),
		m_AsteroidModelMatrices(new glm::mat4[m_AsteroidCount]),
		m_CameraPos(glm::vec3(-10.0f, 40.0f, 100.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_ModelShader(new Shader("res/shaders/BasicModel.shader")),
		m_ModelShaderInstanced(new Shader("res/shaders/BasicModelInstanced.shader")),
		m_UsingInstancing(true),
		// Skybox data
		m_SkyboxShader(new Shader("res/shaders/Skybox.shader")),
		m_VA_Skybox(new VertexArray())
	{
		instance = this;

		float skyboxVertices[] = {
			//   Positions        
			   1.0f,  1.0f, -1.0f,
			  -1.0f, -1.0f, -1.0f,
			  -1.0f,  1.0f, -1.0f,
			   1.0f, -1.0f, -1.0f,

			  -1.0f, -1.0f,  1.0f,
			   1.0f,  1.0f,  1.0f,
			  -1.0f,  1.0f,  1.0f,
			   1.0f, -1.0f,  1.0f,

			  -1.0f, -1.0f, -1.0f,
			  -1.0f,  1.0f,  1.0f,
			  -1.0f,  1.0f, -1.0f,
			  -1.0f, -1.0f,  1.0f,

			   1.0f, -1.0f,  1.0f,
			   1.0f,  1.0f, -1.0f,
			   1.0f,  1.0f,  1.0f,
			   1.0f, -1.0f, -1.0f,

			   1.0f,  1.0f,  1.0f,
			  -1.0f,  1.0f, -1.0f,
			  -1.0f,  1.0f,  1.0f,
			   1.0f,  1.0f, -1.0f,

			  -1.0f, -1.0f,  1.0f,
			   1.0f, -1.0f, -1.0f,
			   1.0f, -1.0f,  1.0f,
			  -1.0f, -1.0f, -1.0f,
		};

		unsigned int skyboxIndices[]{
			0, 1, 2,
			3, 1, 0,

			4, 5, 6,
			7, 5, 4,

			8, 9, 10,
			11, 9, 8,

			12, 13, 14,
			15, 13, 12,

			16, 17, 18,
			19, 17, 16,

			20, 21, 22,
			23, 21, 20
		};

		// Skybox Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Skybox = new VertexBuffer(skyboxVertices, 3 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout skyboxVBLayout;
		skyboxVBLayout.Push<float>(3); // Vertex positions,  vec3
		m_VA_Skybox->AddBuffer(*m_VB_Skybox, skyboxVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Skybox = new IndexBuffer(skyboxIndices, 6 * 6);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestInstancedRendering::~TestInstancedRendering()
	{
	}

	void TestInstancedRendering::OnUpdate(float deltaTime)
	{
	}

	void TestInstancedRendering::OnRender()
	{
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process keyboard inputs
		processInputInstancedRendering(m_MainWindow);

		Renderer renderer;
		// Set per-frame uniforms
		m_ModelShader->Bind();

		// Planet's Model, View, Projection matrices
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		float planetRotation = sin(glfwGetTime() / 100.0f);
		modelMatrix = glm::rotate(modelMatrix, planetRotation, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(20.0f));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 300.0f);
		m_ModelShader->SetMatrix4f("model", modelMatrix);
		m_ModelShader->SetMatrix4f("view", viewMatrix);
		m_ModelShader->SetMatrix4f("proj", projMatrix);
		//
		// Render the planet
		m_PlanetModel->Draw(m_ModelShader);

		if (m_UsingInstancing) 
		{
			// Using instanced rendering
			//
			m_ModelShaderInstanced->Bind();
			// Asteroid's View and Projection matrices
			m_ModelShaderInstanced->SetMatrix4f("view", viewMatrix);
			m_ModelShaderInstanced->SetMatrix4f("proj", projMatrix);
			// Bind the asteroid texture then render all asteroids
			m_AsteroidTexture->Bind(0);
			m_ModelShaderInstanced->SetInt("texture_diffuse0", 0);
			m_AsteroidModel->DrawInstanced(m_ModelShader, m_AsteroidCount);
		}
		else
		{
			// Not using instanced rendering
			//
			// Bind the asteroid texture then render all asteroids
			m_AsteroidTexture->Bind(0);
			m_ModelShader->SetInt("texture_diffuse0", 0);
			// Asteroid translations
			for (int i = 0; i < m_AsteroidCount; i++)
			{
				modelMatrix = m_AsteroidModelMatrices[i];
				// TODO asteroid movement
				//modelMatrix = glm::rotate(modelMatrix, (float)(sin(glfwGetTime()) + 1.0f / 2.0f), glm::vec3(1.0f, 0.0, 0.0f));
				//modelMatrix = glm::rotate(modelMatrix, (float)(cos(glfwGetTime()) + 1.0f / 2.0f), glm::vec3(0.0f, 1.0, 0.0f));
				//modelMatrix = glm::rotate(modelMatrix, (float)(cos(glfwGetTime()) + 1.0f / 2.0f), glm::vec3(0.0f, 0.0, 1.0f));
				m_ModelShader->SetMatrix4f("model", modelMatrix);
				m_AsteroidModel->Draw(m_ModelShader);
			}
		}
		

		// Then render the skybox with depth testing at LEQUAL (and set z component to be (w / w) = 1.0 = max depth in vertex shader)
		glDepthFunc(GL_LEQUAL);
		m_SkyboxShader->Bind();
		// Model, View, Projection matrices
		modelMatrix = glm::mat4(1.0f);
		viewMatrix = glm::mat4(glm::mat3(m_Camera.GetViewMatrix())); // Gets rid of any translations for the skybox
		m_SkyboxShader->SetMatrix4f("modelMatrix", modelMatrix);
		m_SkyboxShader->SetMatrix4f("viewMatrix", viewMatrix);
		m_SkyboxShader->SetMatrix4f("projMatrix", projMatrix);
		renderer.DrawTriangles(*m_VA_Skybox, *m_IB_Skybox, *m_SkyboxShader);
		glDepthFunc(GL_LESS);
	}

	void TestInstancedRendering::OnActivated()
	{
		// Only want to load the models the first time we activate this OpenGL test
		if (!modelsLoaded)
		{
			// Flip textures along y axis before loading
			stbi_set_flip_vertically_on_load(true);
			m_PlanetModel = new Model((char*)"res/models/planet/planet.obj");
			//m_PlanetModel = new Model((char*)"res/models/backpack/backpack.obj");
			m_AsteroidModel = new Model((char*)"res/models/rock/rock.obj");
			modelsLoaded = true;
		}

		// Populate m_AsteroidModelMatrices array
		srand(glfwGetTime()); // initialize random seed	
		float radius = 125.0;
		float offset = 25.0f;
		for (unsigned int i = 0; i < m_AsteroidCount; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);

			// 1. translation: displace along circle with 'radius' in range [-offset, offset]
			float angle = (float)i / (float)m_AsteroidCount * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. scale: scale between 0.05 and 0.25f
			float scale = (rand() % 20) / 100.0f + 0.05;
			model = glm::scale(model, glm::vec3(scale));

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. now add to list of matrices
			m_AsteroidModelMatrices[i] = model;
		}

		// Instanced rendering:
		// Initialize instanced array of model matrices
		//
		// Create a buffer to hold all mat4 models
		unsigned int buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		// Fill buffer with the data
		glBufferData(GL_ARRAY_BUFFER, m_AsteroidCount * sizeof(glm::mat4), &m_AsteroidModelMatrices[0], GL_STATIC_DRAW);
		// For each mesh in the asteroid model, get its Vertex Array and setup attribute pointers
		std::vector<Mesh> asteroidMeshes = m_AsteroidModel->GetMeshes();
		for (unsigned int i = 0; i < asteroidMeshes.size(); i++)
		{
			unsigned int VAO = asteroidMeshes[i].GetVAO();
			glBindVertexArray(VAO);
			std::size_t vec4Size = sizeof(glm::vec4);
			// Maximum attribute size is vec4, so we must use 4 of them to store each mat4
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
			// The attrib divisor tells the vertex shader when to update the instance attribute
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);

			glBindVertexArray(0);
		}

		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		//
		// Skybox shader
		delete m_SkyboxTexture;
		m_SkyboxTexture = new Texture(std::vector<std::string>({ "res/textures/night_skybox_alt/right.jpg",
																 "res/textures/night_skybox_alt/left.jpg",
																 "res/textures/night_skybox_alt/top.jpg",
																 "res/textures/night_skybox_alt/bottom.jpg",
																 "res/textures/night_skybox_alt/front.jpg",
																 "res/textures/night_skybox_alt/back.jpg" }));
		m_SkyboxShader->Bind();
		m_SkyboxTexture->BindCubemap(4);
		m_SkyboxShader->SetInt("u_SkyboxTexture", 4);
		
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackInstancedRendering);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackInstancedRendering);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackInstancedRendering);
	}

	void scroll_callbackInstancedRendering(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestInstancedRendering* instancedRenderingTest = test::TestInstancedRendering::GetInstance();
		Camera* instancedRenderingCamera = instancedRenderingTest->GetCamera();
		instancedRenderingCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackInstancedRendering(GLFWwindow* window, double xpos, double ypos)
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

		test::TestInstancedRendering* instancedRenderingTest = test::TestInstancedRendering::GetInstance();
		Camera* instancedRenderingCamera = instancedRenderingTest->GetCamera();
		instancedRenderingCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackInstancedRendering(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestInstancedRendering* instancedRenderingTest = test::TestInstancedRendering::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputInstancedRendering(GLFWwindow* window) {
		test::TestInstancedRendering* instancedRenderingTest = test::TestInstancedRendering::GetInstance();
		Camera* instancedRenderingCamera = instancedRenderingTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			instancedRenderingCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			instancedRenderingCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			instancedRenderingCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			instancedRenderingCamera->ProcessKeyboard(RIGHT, deltaTime);

		// Toggle on/off instanced rendering mode
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			instancedRenderingTest->ToggleInstancedRendering(false);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			instancedRenderingTest->ToggleInstancedRendering(true);
	}

	void TestInstancedRendering::OnImGuiRender()
	{
		// ImGui interface
		if (m_UsingInstancing)
			ImGui::Text("PRESS 3: Turn OFF instanced rendering");
		else
			ImGui::Text("PRESS 4: Turn ON instanced rendering");
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		
	}
}

