#include "TestManualFramebuffers.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackFramebufferTest(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackFramebufferTest(GLFWwindow* window, double xOffset, double yOffset);
	void processInputFramebufferTest(GLFWwindow* window);
	void mouse_button_callbackFramebufferTest(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestManualFramebuffer* TestManualFramebuffer::instance;

	TestManualFramebuffer::TestManualFramebuffer(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 3.0f)),
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_FBO(new FrameBuffer()),
		m_RenderBufferID(-1),
		m_VA_Ground(new VertexArray()),
		m_VB_Ground(new VertexBuffer()),
		m_IB_Ground(new IndexBuffer()),
		m_VA_Quad(new VertexArray()),
		m_VB_Quad(new VertexBuffer()),
		m_IB_Quad(new IndexBuffer()),
		m_Shader(new Shader("res/shaders/Basic.shader")),
		m_QuadShader(new Shader("res/shaders/FramebufferTest.shader")),
		m_WaterTexture(new Texture("res/textures/shallow_water_texture.png"))
		//m_WaterTexture(new Texture("res/textures/pool_water_texture.png"))
	{
		instance = this;

		// Create vertices and indices
		float groundQuadVertices[] = {
			//       positions      --     tex coords     --    normals
				-800.0, -5.0, -800.0,      0.0,    100.0,      0.0, 1.0, 0.0,
				 800.0, -5.0,  800.0,      100.0,    0.0,      0.0, 1.0, 0.0,
				-800.0, -5.0,  800.0,      0.0,      0.0,      0.0, 1.0, 0.0,
				 800.0, -5.0, -800.0,      100.0,  100.0,      0.0, 1.0, 0.0,
		};

		unsigned int groundQuadIndices[]{
			0, 2, 1,
			3, 0, 1,
		};

		float framebufferQuadVertices[] = {
			//   positions  --   tex coords     
				-0.9,  0.5,      0.0,  0.0,
				-0.5,  0.9,      1.0,  1.0,
				-0.9,  0.9,      0.0,  1.0,
				-0.5,  0.5,      1.0,  0.0,
		};

		unsigned int framebufferQuadIndices[]{
			0, 1, 2,
			3, 0, 1,
		};

		// Ground Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Ground = new VertexBuffer(groundQuadVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVBLayout;
		groundVBLayout.Push<float>(3); // Vertex position,vec3
		groundVBLayout.Push<float>(2); // Texture coordinates, vec2
		groundVBLayout.Push<float>(3); // Normals, vec3
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Ground = new IndexBuffer(groundQuadIndices, 6);

		// Framebuffer quad Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Quad = new VertexBuffer(framebufferQuadVertices, 5 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout framebufferQuadVBLayout;
		framebufferQuadVBLayout.Push<float>(2); // Vertex position,vec3
		framebufferQuadVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_Quad->AddBuffer(*m_VB_Quad, framebufferQuadVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Quad = new IndexBuffer(framebufferQuadIndices, 6);

		// Bind the manually created framebuffer object
		m_FBO->Bind();
		//
		// Next, create and attach any framebuffer attachments (colour/depth/stecil buffers and others)
		// Create an empty 'texture' the same size as the window, to attach to the framebuffer & to render into
		GLCall(glGenTextures(1, &m_FramebufferTexture));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_FramebufferTexture));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL)); // TODO use GL_RGBA?
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Attach empty texture to the active framebuffer
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FramebufferTexture, 0));
		//
		// Create a framebuffer texture containing both depth buffer (24 bits) and stencil buffer (8 bits) information
		/*glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT, 0,
					  GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL );
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebufferTexture, 0);*/
		// 
		// Renderbuffer objects store all the render data directly into their buffer without any conversions to texture-specific formats, 
		// making them faster as a writeable storage medium. However, you cannot read from them directly (can with conversions but is slow).
		// "When we're not sampling from the buffer, a renderbuffer object is generally preferred."
		GLCall(glGenRenderbuffers(1, &m_RenderBufferID));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_RenderBufferID));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT));
		// Attach renderbuffer to the active framebuffer
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_RenderBufferID));

		// Check if Framebuffer is complete before continuing
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "[ERROR] Framebuffer is not complete!" << std::endl;
			ASSERT(0);
		}

		// Return to default framebuffer
		m_FBO->Unbind();

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestManualFramebuffer::~TestManualFramebuffer()
	{
	}

	void TestManualFramebuffer::OnUpdate(float deltaTime)
	{
	}

	void TestManualFramebuffer::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.5f;

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process WASD keyboard camera movement
		processInputFramebufferTest(m_MainWindow);

		Renderer renderer;

		// Bind shader and set its per frame uniforms
		m_Shader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		glm::mat4 proj = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_Camera.Yaw   -= 180.0f; // rotate the camera's yaw 180 degrees around
		m_Camera.Pitch -= 180.0f; // rotate the camera's pitch 180 degrees around
		m_Camera.ProcessMouseMovement(0, 0, false); // call this to make sure it updates its camera vectors, note pitch constraints disabled
		glm::mat4 view = m_Camera.GetViewMatrix();
		glm::mat4 MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);

		// Render to the manually created framebuffer object 
		// Bind it
		m_FBO->Bind();
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer
		glEnable(GL_DEPTH_TEST);
		m_Shader->Bind();
		m_WaterTexture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		m_Shader->SetInt("u_Texture0", 0);
		m_Shader->SetInt("u_ActiveTexture", 0);
		renderer.Draw(*m_VA_Ground, *m_IB_Ground, *m_Shader); // Draws to manual framebuffer

		// Now render the scene to the default framebuffer and use the rendered framebuffer as a texture 
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // rebind default framebuffer
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer

		m_QuadShader->Bind();
		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_FramebufferTexture));
		m_QuadShader->SetInt("framebufferTexture", 1);
		renderer.Draw(*m_VA_Quad, *m_IB_Quad, *m_QuadShader); // Draws the quad to the default framebuffer

		// Now reset to normal ground texture and draw the ground to the default framebuffer
		m_Shader->Bind();
		m_Shader->SetInt("u_ActiveTexture", 0);
		// Rotate the camera's direction back to the front
		m_Camera.Yaw   += 180.0f; 
		m_Camera.Pitch += 180.0f; 
		m_Camera.ProcessMouseMovement(0, 0, true);
		view = m_Camera.GetViewMatrix(); 
		MVP_matrix = proj * view * modelMatrix;
		m_Shader->SetMatrix4f("u_MVP", MVP_matrix);
		renderer.Draw(*m_VA_Ground, *m_IB_Ground, *m_Shader);
	}

	void TestManualFramebuffer::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestManualFramebuffer::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Bind shader program and set uniforms
		m_Shader->Bind();
		m_WaterTexture->Bind(0); // make sure this texture slot is the same as the one set in the next line, which tells the shader where to find the Sampler2D data
		// Set texture to repeat
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		m_Shader->SetInt("u_Texture0", 0);
		m_Shader->SetInt("u_ActiveTexture", 0);

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputFramebufferTest(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackFramebufferTest);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackFramebufferTest);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackFramebufferTest);
	}

	void scroll_callbackFramebufferTest(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestManualFramebuffer* framebufferTest = test::TestManualFramebuffer::GetInstance();
		Camera* framebufferCamera = framebufferTest->GetCamera();
		framebufferCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackFramebufferTest(GLFWwindow* window, double xpos, double ypos)
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

		test::TestManualFramebuffer* framebufferTest = test::TestManualFramebuffer::GetInstance();
		Camera* framebufferCamera = framebufferTest->GetCamera();
		framebufferCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackFramebufferTest(GLFWwindow* window, int button, int action, int mods)
	{
		test::TestManualFramebuffer* framebufferTest = test::TestManualFramebuffer::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputFramebufferTest(GLFWwindow* window) {
		test::TestManualFramebuffer* framebufferTest = test::TestManualFramebuffer::GetInstance();
		Camera* framebufferCamera = framebufferTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			framebufferCamera->ProcessKeyboardForWalkingView(FORWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			framebufferCamera->ProcessKeyboardForWalkingView(BACKWARD, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			framebufferCamera->ProcessKeyboardForWalkingView(LEFT, deltaTime, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			framebufferCamera->ProcessKeyboardForWalkingView(RIGHT, deltaTime, -0.2f);
	}
}

