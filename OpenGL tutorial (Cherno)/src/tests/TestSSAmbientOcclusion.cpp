#include "TestSSAmbientOcclusion.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"
#include <vendor\stb_image\stb_image.h>
#include <random>

namespace test
{
	// Function declarations
	void mouse_callbackSSAO(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackSSAO(GLFWwindow* window, double xOffset, double yOffset);
	void processInputSSAO(GLFWwindow* window);
	void mouse_button_callbackSSAO(GLFWwindow* window, int button, int action, int mods);

	// Init static variable
	TestSSAO* TestSSAO::instance;

	TestSSAO::TestSSAO(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		modelLoaded(false),
		m_Model(nullptr),
		m_GBufferShader(new Shader("res/shaders/GBufferSSAO.shader")),
		m_QuadShader(new Shader("res/shaders/SSAOQuad.shader")),
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png")),
		m_SecondaryTexture(new Texture("res/textures/metal_scratched_texture.png")),
		m_CameraPos(glm::vec3(0.0f, 0.0f, 5.0f)),
		m_CameraFront(glm::vec3(0.0f, 0.0f, -1.0f)),
		m_CameraUp(glm::vec3(0.0f, 1.0f, 0.0f)),
		m_Camera(Camera(m_CameraPos, 60.0f)),
		// Deferred Rendering variables
		m_GBufferSSAO(new FrameBuffer()),
		m_RenderBufferID(-1),
		m_PositionGBuffer(-1),
		m_NormalGBuffer(-1),
		m_AlbedoSpecGBuffer(-1),
		// Ambient Occlusion variables
		m_MaxSamples(64),
		m_NoiseTexture(-1)
	{
		instance = this;

		// Create vertice positions
		float groundVertices[] = {
			//       positions         --     normals    --    tex coords    
				 -800.0, -5.0, -800.0,    0.0, 1.0, 0.0,	    0.0, 100.0,
				  800.0, -5.0,  800.0,    0.0, 1.0, 0.0,	  100.0,   0.0,
				 -800.0, -5.0,  800.0,    0.0, 1.0, 0.0,	    0.0,   0.0,
				  800.0, -5.0, -800.0,    0.0, 1.0, 0.0,	  100.0, 100.0,
		};

		unsigned int groundIndices[]{
			0, 2, 1,
			3, 0, 1,
		};

		float framebufferQuadVertices[] = {
			//   positions  --   tex coords     
				-1.0,  -1.0,      0.0,  0.0,
				 1.0,  1.0,       1.0,  1.0,
				-1.0,  1.0,       0.0,  1.0,
				 1.0,  -1.0,      1.0,  0.0,
		};

		unsigned int framebufferQuadIndices[] = {
			0, 1, 2,
			3, 1, 0,
		};

		// Ground Vertex Array setup
		m_VA_Ground = new VertexArray();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Ground = new VertexBuffer(groundVertices, 8 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout groundVertexBufferLayout;
		groundVertexBufferLayout.Push<float>(3); // Vertex position,	 vec3
		groundVertexBufferLayout.Push<float>(3); // Normals,			 vec3
		groundVertexBufferLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_Ground->AddBuffer(*m_VB_Ground, groundVertexBufferLayout);
		// Init index buffer and bind to Vertex Array
		m_IB_Ground = new IndexBuffer(groundIndices, 6);

		// Framebuffer quad Vertex Array setup
		m_VA_Quad = new VertexArray();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Quad = new VertexBuffer(framebufferQuadVertices, 4 * 4 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout framebufferQuadVBLayout;
		framebufferQuadVBLayout.Push<float>(2); // Vertex position,		vec2
		framebufferQuadVBLayout.Push<float>(2); // Texture coordinates, vec2
		m_VA_Quad->AddBuffer(*m_VB_Quad, framebufferQuadVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Quad = new IndexBuffer(framebufferQuadIndices, 6);
	}

	TestSSAO::~TestSSAO()
	{
	}

	void TestSSAO::OnUpdate(float deltaTime)
	{
	}

	void TestSSAO::OnRender()
	{
		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process WASD keyboard camera movement
		processInputSSAO(m_MainWindow);
		Renderer renderer;
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 2.0f;

		// Bind manually created framebuffer with the special attachment components that we want to write to
		m_GBufferSSAO->Bind();
		GLCall(glClearColor(clearColour[0] / darknessFactor,
							clearColour[1] / darknessFactor,
							clearColour[2] / darknessFactor,
							clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Bind shader and set any 'per frame' uniforms
		m_GBufferShader->Bind();
		//
		// Create model, view, projection matrices 
		// Send combined MVP matrix to shader
		glm::mat4 modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_GBufferShader->SetMatrix4f("model", modelMatrix);
		m_GBufferShader->SetMatrix4f("view", viewMatrix);
		m_GBufferShader->SetMatrix4f("proj", projMatrix);
		// Bind ground diffuse texture
		m_GroundTexture->BindAndSetRepeating(0);
		m_GBufferShader->SetUniform1i("texture_diffuse0", 0);
		// Bind ground specular texture
		m_GroundTexture->BindAndSetRepeating(1); // hack for now -- just using the same texture here
		m_GBufferShader->SetUniform1i("texture_specular0", 1);
		// Render the ground
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_GBufferShader);
		// Load model's uniforms and render the loaded backpack models
		m_Model->Draw(m_GBufferShader);

		// At this point, the GBuffer has been filled with all necessary information for SSAO (Screen-Space Ambient Occlusion)


		// Next render the scene to the default framebuffer and use the rendered framebuffer as a texture
		// Rebind default framebuffer
		m_GBufferSSAO->Unbind();
		// Clear depth buffer and colour buffer attachments
		GLCall(glClearColor(clearColour[0] / darknessFactor,
							clearColour[1] / darknessFactor,
							clearColour[2] / darknessFactor,
							clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer

		// Next take its four buffers (world position, normal, albedo, specular) and run a single lighting fragment shader for all lights in the scene
		m_QuadShader->Bind();
		m_QuadShader->SetVec3("viewPos", m_Camera.Position);
		// Bind all three GBuffer attachments to the sampler2D uniforms
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PositionGBuffer));
		m_QuadShader->SetInt("gPosition", 0);
		GLCall(glActiveTexture(GL_TEXTURE1));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_NormalGBuffer));
		m_QuadShader->SetInt("gNormal", 1);
		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_AlbedoSpecGBuffer));
		m_QuadShader->SetInt("gAlbedoSpec", 2);
		// Draw the completed lighting effects textured quad to the default framebuffer
		renderer.DrawTriangles(*m_VA_Quad, *m_IB_Quad, *m_QuadShader);
	}

	void TestSSAO::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestSSAO::OnActivated()
	{
		// Only want to load the model the first time we activate this OpenGL test
		if (!modelLoaded)
		{
			// Flip texture along y axis before loading
			stbi_set_flip_vertically_on_load(true);
			m_Model = new Model((char*)"res/models/backpack/backpack.obj");
			//m_Model = new Model((char*)"res/models/donut tutorial/donut_icing.obj");
			//m_Model = new Model((char*)"res/models/donut tutorial/coffee_cup.obj");
			modelLoaded = true;
		}

		// Setup manual framebuffer (GBuffer)
		m_GBufferSSAO->Bind();
		// Position 'colour' buffer
		glGenTextures(1, &m_PositionGBuffer);
		glBindTexture(GL_TEXTURE_2D, m_PositionGBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PositionGBuffer, 0);
		// Normal 'colour' buffer
		glGenTextures(1, &m_NormalGBuffer);
		glBindTexture(GL_TEXTURE_2D, m_NormalGBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_NormalGBuffer, 0);
		// Albedo + Specular 'colour' buffer
		glGenTextures(1, &m_AlbedoSpecGBuffer);
		glBindTexture(GL_TEXTURE_2D, m_AlbedoSpecGBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_AlbedoSpecGBuffer, 0);
		// Tell OpenGL to use the above three colour attachments for rendering 
		unsigned int gBufferAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, gBufferAttachments);
		// Then also add renderbuffer object as depth/stencil buffer
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
			std::cout << "[ERROR] Manual framebuffer is not complete!" << std::endl;
			ASSERT(0);
		}

		// Ambient Occlusion hemisphere sampling kernel setup
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		for (unsigned int i = 0; i < m_MaxSamples; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0, // x range [-1.0, 1.0]
				randomFloats(generator) * 2.0 - 1.0, // y range [-1.0, 1.0]
				randomFloats(generator)				 // z range [ 0.0, 1.0] (since we're sampling a hemisphere)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator); 
			// Linear interpolate to make most samples closer to the original fragment position
			float scale = (float) i / m_MaxSamples;
			sample *= lerp(0.1f, 1.0f, scale * scale);
			ssaoKernel.push_back(sample);
		}
		// Random kernel rotations 
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
							randomFloats(generator) * 2.0 - 1.0,
							0.0f);
			ssaoNoise.push_back(noise);
		}
		// Generate repeating 4x4 texture of above noise kernel rotations
		glGenTextures(1, &m_NoiseTexture);
		glBindTexture(GL_TEXTURE_2D, m_NoiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Enable face culling
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // cull back faces
		glFrontFace(GL_CCW); // tell OpenGL that front faces have CCW winding order

		// Bind shader program and set uniforms
		m_GBufferShader->Bind();
		// Reset matrices on activation
		glm::mat4 modelMatrix = glm::mat4(1.0);
		//modelMatrix = glm::translate(modelMatrix, glm::vec3(50.0, 0.0, 36.0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0));
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 200.0f);
		m_GBufferShader->SetMatrix4f("model", modelMatrix);
		m_GBufferShader->SetMatrix4f("view", viewMatrix);
		m_GBufferShader->SetMatrix4f("proj", projMatrix);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);
		// Enable blending
		GLCall(glEnable(GL_BLEND));
		//GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glBlendFunc(GL_ONE, GL_ZERO));

		// Reset all callbacks
		// Callback function for keyboard inputs
		processInputSSAO(m_MainWindow);
		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackSSAO);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackSSAO);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackSSAO);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);
		// Enable blending
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_ONE, GL_ZERO));
	}

	void scroll_callbackSSAO(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();
		ssaoCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackSSAO(GLFWwindow* window, double xpos, double ypos)
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

		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();
		ssaoCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackSSAO(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputSSAO(GLFWwindow* window) {
		test::TestSSAO* ssaoTest = test::TestSSAO::GetInstance();
		Camera* ssaoCamera = ssaoTest->GetCamera();

		// Camera position movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			ssaoCamera->ProcessKeyboard(RIGHT, deltaTime);
	}

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}
}

