#include "TestHDRBloom.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackHDRBloom(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackHDRBloom(GLFWwindow* window, double xOffset, double yOffset);
	void processInputHDRBloom(GLFWwindow* window);
	void mouse_button_callbackHDRBloom(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestHDRBloom* TestHDRBloom::instance;

	TestHDRBloom::TestHDRBloom(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(0.0f, 0.0f, -3.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f, m_CameraUp, 90.0f, 0.0f)),
		m_ManualFramebuffer(new FrameBuffer()),
		m_VA_Ground(new VertexArray()),
		m_VB_Ground(new VertexBuffer()),
		m_IB_Ground(new IndexBuffer()),
		m_VA_Quad(new VertexArray()),
		m_VB_Quad(new VertexBuffer()),
		m_IB_Quad(new IndexBuffer()),
		m_VA_Cube(new VertexArray()),
		m_VB_Cube(new VertexBuffer()),
		m_IB_Cube(new IndexBuffer()),
		m_HDRLightingShader(new Shader("res/shaders/HDRBloomSetup.shader")),
		m_QuadShader(new Shader("res/shaders/HDRBloom.shader")),
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png")),
		//m_CubeTexture(new Texture("res/textures/wooden_container_texture.png"))
		m_CubeTexture(new Texture("res/textures/metal_border_container_texture.png"))
	{
		instance = this;

		// Create vertices and indices
		float groundVertices[] = {
			//       positions      --     tex coords     --    normals
				-800.0, -5.0, -800.0,      0.0,    100.0,      0.0, 1.0, 0.0,
				 800.0, -5.0,  800.0,      100.0,    0.0,      0.0, 1.0, 0.0,
				-800.0, -5.0,  800.0,      0.0,      0.0,      0.0, 1.0, 0.0,
				 800.0, -5.0, -800.0,      100.0,  100.0,      0.0, 1.0, 0.0,
		};

		unsigned int groundIndices[] = {
			0, 2, 1,
			3, 0, 1,
		};

		float framebufferQuadVertices[] = {
			//   positions  --   tex coords     
				-0.9,  -0.9,      0.0,  0.0,
				 0.9,  0.9,       1.0,  1.0,
				-0.9,  0.9,       0.0,  1.0,
				 0.9,  -0.9,      1.0,  0.0,
		};

		unsigned int framebufferQuadIndices[] = {
			0, 1, 2,
			3, 1, 0,
		};

		float cubeVertices[] = {
			// positions      --  tex coords   --  normals
			   0.5,  0.5, -0.5,    1.0, 1.0,    0.0, 0.0, -1.0, // Cube back
			  -0.5, -0.5, -0.5,    0.0, 0.0,    0.0, 0.0, -1.0, 
			  -0.5,  0.5, -0.5,    0.0, 1.0,    0.0, 0.0, -1.0, 
			   0.5, -0.5, -0.5,    1.0, 0.0,    0.0, 0.0, -1.0, 

			  -0.5, -0.5,  0.5,    0.0, 0.0,    0.0, 0.0, 1.0, // Cube front
			   0.5,  0.5,  0.5,    1.0, 1.0,    0.0, 0.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,    0.0, 0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,    0.0, 0.0, 1.0,

			  -0.5, -0.5, -0.5,    0.0, 0.0,    -1.0, 0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,    -1.0, 0.0, 0.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,    -1.0, 0.0, 0.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,    -1.0, 0.0, 0.0,

			   0.5, -0.5,  0.5,    0.0, 0.0,    1.0, 0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,    1.0, 0.0, 0.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,    1.0, 0.0, 0.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,    1.0, 0.0, 0.0,

			   0.5,  0.5,  0.5,    0.0, 0.0,    0.0, 1.0, 0.0, // Cube top
			  -0.5,  0.5, -0.5,    1.0, 1.0,    0.0, 1.0, 0.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,    0.0, 1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,    0.0, 1.0, 0.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0,    0.0, -1.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,    0.0, -1.0, 0.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,    0.0, -1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,    0.0, -1.0, 0.0,
		};

		unsigned int cubeIndices[] = {
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
		

		// Ground Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Ground = new VertexBuffer(groundVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVBLayout;
		groundVBLayout.Push<float>(3); // Vertex position,		vec3
		groundVBLayout.Push<float>(2); // Texture coordinates,	vec2
		groundVBLayout.Push<float>(3); // Normals,				vec3
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Ground = new IndexBuffer(groundIndices, 6);

		// Cube Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Cube = new VertexBuffer(cubeVertices, 8 * 24 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout cubeVBLayout;
		cubeVBLayout.Push<float>(3); // Vertex position,	 vec3
		cubeVBLayout.Push<float>(2); // Texture coordinates, vec2
		cubeVBLayout.Push<float>(3); // Normals,			 vec3
		m_VA_Cube->AddBuffer(*m_VB_Cube, cubeVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Cube = new IndexBuffer(cubeIndices, 6 * 6);

		// Framebuffer quad Vertex Array setup
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Quad = new VertexBuffer(framebufferQuadVertices, 5 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout framebufferQuadVBLayout;
		framebufferQuadVBLayout.Push<float>(2); // Vertex position,		vec2
		framebufferQuadVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_Quad->AddBuffer(*m_VB_Quad, framebufferQuadVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Quad = new IndexBuffer(framebufferQuadIndices, 6);

		// Bind the manually created framebuffer object
		m_ManualFramebuffer->Bind();
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
		m_ManualFramebuffer->Unbind();

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestHDRBloom::~TestHDRBloom()
	{
	}

	void TestHDRBloom::OnUpdate(float deltaTime)
	{
	}

	void TestHDRBloom::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.5f;

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process WASD keyboard camera movement
		processInputHDRBloom(m_MainWindow);

		Renderer renderer;

		// Bind shader and set its per frame uniforms
		m_HDRLightingShader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 model = glm::mat4(1.0);
		glm::mat4 view = m_Camera.GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_HDRLightingShader->SetMatrix4f("model", model);
		m_HDRLightingShader->SetMatrix4f("view", view);
		m_HDRLightingShader->SetMatrix4f("proj", proj);
		m_HDRLightingShader->SetVec3("viewPos", m_Camera.Position);

		// Render to the manually created framebuffer object 
		// Bind it
		m_ManualFramebuffer->Bind();
		// Clear depth buffer and colour buffer attachments
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer
		glEnable(GL_DEPTH_TEST);
		m_HDRLightingShader->Bind();
		// Draw ground to manual framebuffer
		m_GroundTexture->BindAndSetRepeating(0);
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_HDRLightingShader); 
		// Draw cube to manual framebuffer
		m_CubeTexture->Bind(0);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_HDRLightingShader);

		// Now render the scene to the default framebuffer and use the rendered framebuffer as a texture 
		// Rebind default framebuffer
		m_ManualFramebuffer->Unbind();
		// Clear depth buffer and colour buffer attachments
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer

		m_QuadShader->Bind();
		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_FramebufferTexture));
		m_QuadShader->SetInt("framebufferTexture", 2);
		// Draw the rear-view framebuffer textured quad to the default framebuffer
		renderer.DrawTriangles(*m_VA_Quad, *m_IB_Quad, *m_QuadShader); 
	}

	void TestHDRBloom::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestHDRBloom::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Bind shader program and set uniforms
		m_HDRLightingShader->Bind();
		m_HDRLightingShader->SetInt("u_Material.diffuse", 0);
		m_HDRLightingShader->SetVec3("u_Material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		m_HDRLightingShader->SetFloat("u_Material.shininess", 2^4);
		// Point light properties
		m_HDRLightingShader->SetVec3("pointLights[0].position", glm::vec3(5.0f, -3.0f, -2.0f));
		m_HDRLightingShader->SetVec3("pointLights[0].ambient", glm::vec3(0.4f));
		m_HDRLightingShader->SetVec3("pointLights[0].diffuse", glm::vec3(1.0f));
		m_HDRLightingShader->SetVec3("pointLights[0].specular", glm::vec3(0.4)); 
		m_HDRLightingShader->SetFloat("pointLights[0].constant", 1.0f); 
		m_HDRLightingShader->SetFloat("pointLights[0].linear", 0.02f); 
		m_HDRLightingShader->SetFloat("pointLights[0].quadratic", 0.01f); 
		m_HDRLightingShader->SetVec3("pointLights[1].position", glm::vec3(0.0f, -4.0f, 10.0f));
		m_HDRLightingShader->SetVec3("pointLights[1].ambient", glm::vec3(0.1f));
		m_HDRLightingShader->SetVec3("pointLights[1].diffuse", glm::vec3(1.0f));
		m_HDRLightingShader->SetVec3("pointLights[1].specular", glm::vec3(0.4));
		m_HDRLightingShader->SetFloat("pointLights[1].constant", 1.0f);
		m_HDRLightingShader->SetFloat("pointLights[1].linear", 0.02f);
		m_HDRLightingShader->SetFloat("pointLights[1].quadratic", 0.01f);

	// Implementing attenuation: f_att = 1.0 / (constant + linear*distance + quadratic*distance^2)
	float constant;
	float linear;
	float quadratic;

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputHDRBloom(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackHDRBloom);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackHDRBloom);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackHDRBloom);
	}

	void scroll_callbackHDRBloom(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestHDRBloom* bloomHDRTest = test::TestHDRBloom::GetInstance();
		Camera* bloomHDRCamera = bloomHDRTest->GetCamera();
		bloomHDRCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackHDRBloom(GLFWwindow* window, double xpos, double ypos)
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

		test::TestHDRBloom* bloomHDRTest = test::TestHDRBloom::GetInstance();
		Camera* bloomHDRCamera = bloomHDRTest->GetCamera();
		bloomHDRCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackHDRBloom(GLFWwindow* window, int button, int action, int mods)
	{
		//test::TestHDRBloom* bloomHDRTest = test::TestHDRBloom::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputHDRBloom(GLFWwindow* window) {
		test::TestHDRBloom* bloomHDRTest = test::TestHDRBloom::GetInstance();
		Camera* bloomHDRCamera = bloomHDRTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			bloomHDRCamera->ProcessKeyboardForWalkingView(FORWARD, deltaTime / 2.0f, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			bloomHDRCamera->ProcessKeyboardForWalkingView(BACKWARD, deltaTime / 2.0f, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			bloomHDRCamera->ProcessKeyboardForWalkingView(LEFT, deltaTime / 2.0f, -0.2f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			bloomHDRCamera->ProcessKeyboardForWalkingView(RIGHT, deltaTime / 2.0f, -0.2f);
	}
}

