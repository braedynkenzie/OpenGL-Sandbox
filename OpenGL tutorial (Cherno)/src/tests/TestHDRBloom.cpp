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
		m_PointlightsShader(new Shader("res/shaders/PointLights.shader")),
		m_QuadShader(new Shader("res/shaders/HDRBloom.shader")),
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png")),
		//m_CubeTexture(new Texture("res/textures/wooden_container_texture.png"))
		//m_CubeTexture(new Texture("res/textures/metal_border_container_texture.png"))
		m_CubeTexture(new Texture("res/textures/brick_texture.png")),
		m_LightIntensity(1.0f),
		m_LightExposure(1.0f),
		m_UsingHDR(true),
		// Bloom colour buffer blur effect shader
		m_BlurShader(new Shader("res/shaders/GaussianBlur.shader")),
		m_NumBlurPasses(20),
		// Skybox data
		m_SkyboxShader(new Shader("res/shaders/Skybox.shader")),
		m_VA_Skybox(new VertexArray())
	{
		instance = this;

		m_PointLightPositions[0] = glm::vec3(5.0f, -3.0f, -2.0f);
		m_PointLightPositions[1] = glm::vec3(0.0f, -4.0f, 10.0f);
		m_PointLightColours[0] = glm::vec3(1.0f, 0.4f, 0.2f);
		m_PointLightColours[1] = glm::vec3(0.2f, 1.0f, 1.0f);
		
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
				-1.0,  -1.0,      0.0,  0.0,
				 1.0,  1.0,       1.0,  1.0,
				-1.0,  1.0,       0.0,  1.0,
				 1.0,  -1.0,      1.0,  0.0,
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

		// Framebuffer setup
		{
			// Bind the manually created framebuffer object
			m_ManualFramebuffer->Bind();
			//
			// Next, create and attach any framebuffer attachments (colour/depth/stecil buffers and others)
			// Create an empty HDR colour buffer for typical rendering using lighting 
			GLCall(glGenTextures(1, &m_HDRBuffer));
			GLCall(glBindTexture(GL_TEXTURE_2D, m_HDRBuffer));
			// Floating point framebuffer attachment with 16 bits per colour component
			// With a floating point colour buffer, we can now render the scene and colour values won't get clamped between 0.0 and 1.0
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Attach empty HDR colour buffer to the active framebuffer
			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_HDRBuffer, 0));
			//
			// Also create a Bloom colour buffer that will just store pixels over a certain brightness level
			GLCall(glGenTextures(1, &m_BloomBuffer));
			GLCall(glBindTexture(GL_TEXTURE_2D, m_BloomBuffer));
			// Floating point framebuffer attachment with 16 bits per colour component
			// With a floating point colour buffer, we can now render the scene and colour values won't get clamped between 0.0 and 1.0
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// Attach empty Bloom colour buffer to the active framebuffer
			GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_BloomBuffer, 0));
			//
			// Tell OpenGL which buffers we are drawing to in the shader (these are the above HDR and Bloom buffers)
			unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, attachments);
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

			// Setup two framebuffers for performing Gaussian blur effect on Bloom buffer output
			GLCall(glGenFramebuffers(2, m_PingpongFramebuffers));
			GLCall(glGenTextures(2, m_PingpongColourBuffers));
			for (unsigned int i = 0; i < 2; i++)
			{
				GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_PingpongFramebuffers[i]));
				GLCall(glBindTexture(GL_TEXTURE_2D, m_PingpongColourBuffers[i]));
				GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingpongColourBuffers[i], 0));
			}
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
		GLCall(glClearColor(clearColour[0] * m_LightIntensity / darknessFactor, 
							clearColour[1] * m_LightIntensity / darknessFactor,
							clearColour[2] * m_LightIntensity / darknessFactor, 
							clearColour[3] * m_LightIntensity / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer
		glEnable(GL_DEPTH_TEST);
		m_HDRLightingShader->Bind();
		// Changing pointlight properties
		// Pointlight #1
		m_HDRLightingShader->SetVec3("pointLights[0].ambient", 0.1f * m_PointLightColours[0] * m_LightIntensity); // + glm::vec3(glm::max(m_LightIntensity - 1.0f, 0.0f)));
		m_HDRLightingShader->SetVec3("pointLights[0].diffuse", 1.0f * m_PointLightColours[0] * m_LightIntensity);
		m_HDRLightingShader->SetVec3("pointLights[0].specular", 0.4f * m_PointLightColours[0] * m_LightIntensity);
		// Pointlight #2
		m_HDRLightingShader->SetVec3("pointLights[1].ambient", 0.1f * m_PointLightColours[1] * m_LightIntensity); // +glm::vec3(glm::max(m_LightIntensity - 1.0f, 0.0f)));
		m_HDRLightingShader->SetVec3("pointLights[1].diffuse", 1.0f * m_PointLightColours[1] * m_LightIntensity);
		m_HDRLightingShader->SetVec3("pointLights[1].specular", 0.4f * m_PointLightColours[1] * m_LightIntensity);
		// Draw ground to manual framebuffer
		m_GroundTexture->BindAndSetRepeating(0);
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_HDRLightingShader); 
		// Draw cube to manual framebuffer
		m_CubeTexture->Bind(0);
		model = glm::translate(model, glm::vec3(7.0f, 0.0f, 5.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		m_HDRLightingShader->SetMatrix4f("model", model);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_HDRLightingShader);
		// Draw the pointlight source cubes to manual framebuffer
		// Pointlight #1
		m_PointlightsShader->Bind();
		m_PointlightsShader->SetMatrix4f("model", glm::translate(glm::mat4(1.0f), m_PointLightPositions[0]));
		m_PointlightsShader->SetMatrix4f("view", view);
		m_PointlightsShader->SetMatrix4f("proj", proj);
		m_PointlightsShader->SetVec3("pointLightColour", m_PointLightColours[0] * m_LightIntensity * 2.0f);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_PointlightsShader);
		// Pointlight #2
		m_PointlightsShader->SetMatrix4f("model", glm::translate(glm::mat4(1.0f), m_PointLightPositions[1]));
		m_PointlightsShader->SetVec3("pointLightColour", m_PointLightColours[1] * m_LightIntensity * 2.0f);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_PointlightsShader);
		// Then render the skybox with depth testing at LEQUAL (and set z component to be (w / w) = 1.0 = max depth in vertex shader)
		glDepthFunc(GL_LEQUAL);
		m_SkyboxShader->Bind();
		// Model, View, Projection matrices
		model = glm::mat4(1.0f);
		view = glm::mat4(glm::mat3(m_Camera.GetViewMatrix())); // Gets rid of any translation
		proj = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_SkyboxShader->SetMatrix4f("modelMatrix", model);
		m_SkyboxShader->SetMatrix4f("viewMatrix", view);
		m_SkyboxShader->SetMatrix4f("projMatrix", proj);
		renderer.DrawTriangles(*m_VA_Skybox, *m_IB_Skybox, *m_SkyboxShader);
		glDepthFunc(GL_LESS);


		// Now that we've filled the HDR and bloom colour buffers, Gaussian blur for the bloom effect
		bool horizontal = true;
		bool first_iteration = true;
		m_BlurShader->Bind();
		for (unsigned int i = 0; i < m_NumBlurPasses; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_PingpongFramebuffers[horizontal]);
			m_BlurShader->SetBool("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? m_BloomBuffer : m_PingpongColourBuffers[!horizontal]);
			renderer.DrawTriangles(*m_VA_Quad, *m_IB_Quad, *m_BlurShader);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Now render the scene to the default framebuffer and use the rendered framebuffer as a texture 
		// Rebind default framebuffer
		m_ManualFramebuffer->Unbind();
		// Clear depth buffer and colour buffer attachments
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // not using the stencil buffer

		m_QuadShader->Bind();
		// Bind both HDR and blurred Bloom colour buffers
		GLCall(glActiveTexture(GL_TEXTURE2));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_HDRBuffer));
		m_QuadShader->SetInt("hdrImageTexture", 2);
		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_PingpongColourBuffers[!horizontal]));
		m_QuadShader->SetInt("bloomImageTexture", 3);
		m_QuadShader->SetBool("u_UsingHDR", m_UsingHDR);
		m_QuadShader->SetFloat("u_Exposure", m_LightExposure);
		// Draw the rear-view framebuffer textured quad to the default framebuffer
		renderer.DrawTriangles(*m_VA_Quad, *m_IB_Quad, *m_QuadShader); 
	}

	void TestHDRBloom::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS '3' to increase light intensity");
		ImGui::Text("PRESS '4' to decrease light intensity");
		ImGui::Text("PRESS '5' to increase exposure");
		ImGui::Text("PRESS '6' to decrease exposure");
		ImGui::Text("PRESS '7' to turn off HDR and Bloom");
		ImGui::Text("PRESS '8' to turn on HDR and Bloom");
		ImGui::Text("PRESS '9' to increase bloom blur effect");
		ImGui::Text("PRESS '0' to decrease bloom blur effect");
		ImGui::Text(" - - - ");
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
		// Constant pointlight properties
		float constant	= 1.0f;
		float linear	= 0.05f;
		float quadratic = 0.02f;
		// Pointlight #1
		m_HDRLightingShader->SetVec3("pointLights[0].position", m_PointLightPositions[0]);
		m_HDRLightingShader->SetFloat("pointLights[0].constant", constant);
		m_HDRLightingShader->SetFloat("pointLights[0].linear", linear);
		m_HDRLightingShader->SetFloat("pointLights[0].quadratic", quadratic);
		// Pointlight #2
		m_HDRLightingShader->SetVec3("pointLights[1].position", m_PointLightPositions[1]);
		m_HDRLightingShader->SetFloat("pointLights[1].constant", constant);
		m_HDRLightingShader->SetFloat("pointLights[1].linear", linear);
		m_HDRLightingShader->SetFloat("pointLights[1].quadratic", quadratic);

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
		// Callback function for keyboard inputs
		processInputHDRBloom(m_MainWindow);
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackHDRBloom);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackHDRBloom);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackHDRBloom);
	}

	void TestHDRBloom::LightIntensity(const int dir)
	{
		if (dir == 1)
		{
			// Increase light intensity
			m_LightIntensity *= 1.01;
		}
		else if (dir == -1)
		{
			// Decrease light intensity
			m_LightIntensity *= 0.99;
		}
	}

	void TestHDRBloom::ExposureLevel(const int dir)
	{
		if (dir == 1)
		{
			// Increase light exposure
			m_LightExposure *= 1.01;
		}
		else if (dir == -1)
		{
			// Decrease light exposure
			m_LightExposure *= 0.99;
		}
	}

	void TestHDRBloom::BloomBlurAmount(const int dir)
	{
		m_NumBlurPasses += dir;
		if (m_NumBlurPasses < 1) m_NumBlurPasses = 1;
	}

	void TestHDRBloom::ToggleHDR(bool flag)
	{
		m_UsingHDR = flag;
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
		// Change light intensity
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			bloomHDRTest->LightIntensity(1);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			bloomHDRTest->LightIntensity(-1);
		// Change exposure levels
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			bloomHDRTest->ExposureLevel(1);
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			bloomHDRTest->ExposureLevel(-1);
		// Toggle HDR
		if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
			bloomHDRTest->ToggleHDR(false);
		if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
			bloomHDRTest->ToggleHDR(true);
		// Change bloom blur amount
		if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
			bloomHDRTest->BloomBlurAmount(1);
		if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
			bloomHDRTest->BloomBlurAmount(-1);
	}
}

