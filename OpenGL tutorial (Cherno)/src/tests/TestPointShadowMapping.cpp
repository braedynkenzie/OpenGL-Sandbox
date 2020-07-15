#include "TestPointShadowMapping.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>
#include "Globals.h"

namespace test
{
	// Function declarations
	void mouse_callbackPointShadowMapping(GLFWwindow* window, double xpos, double ypos);
	void scroll_callbackPointShadowMapping(GLFWwindow* window, double xOffset, double yOffset);
	void processInputPointShadowMapping(GLFWwindow* window);
	void mouse_button_callbackPointShadowMapping(GLFWwindow* window, int button, int action, int mods);

	// Fill in 'tangent' and 'bitangent' vectors given the edge and deltaUV values
	void CalculateTangentBitangent(glm::vec3& tangent, glm::vec3& bitangent, glm::vec3& edge1, glm::vec3& edge2, glm::vec2& deltaUV1, glm::vec2& deltaUV2)
	{
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	}

	// Init static variable
	TestPointShadowMapping* TestPointShadowMapping::instance;

	TestPointShadowMapping::TestPointShadowMapping(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow),
		m_CameraPos(glm::vec3(4.0f, 8.0f, 16.0f)),
		m_Camera(Camera(m_CameraPos, 75.0f)),
		m_Shader(new Shader("res/shaders/BasicShadowMapping.shader")),
		m_ShadowDepthMapShader(new Shader("res/shaders/ShadowMapping.shader")),
		m_ContainerTexture(new Texture("res/textures/metal_border_container_texture.png", false)),
		//m_BrickTexture(new Texture("res/textures/brick_texture.png", false)),
		m_BrickTexture(new Texture("res/textures/metal_border_container_texture.png", false)),
		m_BrickNormalMap(new Texture("res/textures/brick_normal_map.png", false)),
		m_UsingNormalMap(true),
		m_GroundTexture(new Texture("res/textures/wooden_floor_texture.png", false)),
		m_VA_Cube(new VertexArray()),
		m_VA_Ground(new VertexArray()),
		// Flashlight properties
		m_IsFlashlightOn(true),
		m_FlashlightColour(glm::vec3(1.0f)), m_FlashlightDiffuseIntensity(glm::vec3(1.0f)),
		m_FlashlightAmbientIntensity(glm::vec3(0.2f)), m_FlashlightSpecularIntensity(glm::vec3(0.6f)),
		m_FlashlightDiffuseColour(m_FlashlightColour* m_FlashlightDiffuseIntensity),
		m_FlashlightAmbientColour(m_FlashlightDiffuseColour* m_FlashlightAmbientIntensity),
		// Directional light properties
		m_DirLightDirection(glm::vec3(-1.0f, 1.0f, 1.0)),
		m_DirLightAmbient(glm::vec3(0.05f, 0.05f, 0.05f)),
		m_DirLightDiffuse(glm::vec3(0.4f, 0.4f, 0.4f)),
		m_DirLightSpecular(glm::vec3(0.1f, 0.09f, 0.1f)), // sunlight specular highlights
		// Shadow map properties
		m_ShadowMapWidth(1024),
		m_ShadowMapHeight(1024),
		m_ShadowDepthMap(0),
		m_DepthMapFBO(0)
	{
		instance = this;

		// Create vertices and incdices
		float groundVertices[] = {
			//       positions      --     tex coords     --    normals
				-800.0, -10.0, -800.0,      0.0, 50.0,      0.0, 1.0, 0.0,
				 800.0, -10.0,  800.0,    50.0,   0.0,      0.0, 1.0, 0.0,
				-800.0, -10.0,  800.0,      0.0,  0.0,      0.0, 1.0, 0.0,
				 800.0, -10.0, -800.0,    50.0,  50.0,      0.0, 1.0, 0.0,
		};

		unsigned int groundIndices[]{
			0, 2, 1,
			3, 0, 1,
		};

		//// Back cube face
		//glm::vec3 back_pos1(0.5, 0.5, -0.5);
		//glm::vec3 back_pos2(-0.5, -0.5, -0.5);
		//glm::vec3 back_pos3(-0.5, 0.5, -0.5);
		//glm::vec3 back_pos4(0.5, -0.5, -0.5);
		//glm::vec2 back_uv1(1.0, 1.0);
		//glm::vec2 back_uv2(0.0, 0.0);
		//glm::vec2 back_uv3(0.0, 1.0);
		//glm::vec2 back_uv4(1.0, 0.0);
		//glm::vec3 back_nm(0.0, 0.0, -1.0);
		//// Calculate tangent/bitangent vectors of both back facing triangles
		//// Triangle 1
		//glm::vec3 back_tangent1;
		//glm::vec3 back_bitangent1;
		//glm::vec3 back_edge1 =    back_pos2 - back_pos1;
		//glm::vec3 back_edge2 =    back_pos3 - back_pos1;
		//glm::vec2 back_deltaUV1 = back_uv2 -  back_uv1;
		//glm::vec2 back_deltaUV2 = back_uv3 -  back_uv1;
		//CalculateTangentBitangent(back_tangent1, 
		//						  back_bitangent1, 
		//						  back_edge1, 
		//						  back_edge2, 
		//						  back_deltaUV1, 
		//						  back_deltaUV2);
		//// Triangle 2
		//glm::vec3 back_tangent2;
		//glm::vec3 back_bitangent2;
		//back_edge1 =    back_pos4 - back_pos2;
		//back_edge2 =    back_pos4 - back_pos1;
		//back_deltaUV1 = back_uv4 -  back_uv2;
		//back_deltaUV2 = back_uv4 -  back_uv1;
		//CalculateTangentBitangent(back_tangent2, 
		//						  back_bitangent2, 
		//						  back_edge1, 
		//						  back_edge2, 
		//						  back_deltaUV1, 
		//						  back_deltaUV2);
		//// Front cube face
		//glm::vec3 front_pos1(-0.5, -0.5, 0.5);
		//glm::vec3 front_pos2( 0.5,  0.5, 0.5);
		//glm::vec3 front_pos3(-0.5,  0.5, 0.5);
		//glm::vec3 front_pos4( 0.5, -0.5, 0.5);
		//glm::vec2 front_uv1(0.0, 0.0);
		//glm::vec2 front_uv2(1.0, 1.0);
		//glm::vec2 front_uv3(0.0, 1.0);
		//glm::vec2 front_uv4(1.0, 0.0);
		//glm::vec3 front_nm(0.0, 0.0, 1.0);
		//// Calculate tangent/bitangent vectors of both front facing triangles
		//// Triangle 1
		//glm::vec3 front_tangent1;
		//glm::vec3 front_bitangent1;
		//glm::vec3 front_edge1 = front_pos2 - front_pos1;
		//glm::vec3 front_edge2 = front_pos3 - front_pos1;
		//glm::vec2 front_deltaUV1 = front_uv2 - front_uv1;
		//glm::vec2 front_deltaUV2 = front_uv3 - front_uv1;
		//CalculateTangentBitangent(front_tangent1, 
		//						  front_bitangent1, 
		//						  front_edge1, 
		//						  front_edge2, 
		//						  front_deltaUV1, 
		//						  front_deltaUV2);
		//// Triangle 2
		//glm::vec3 front_tangent2;
		//glm::vec3 front_bitangent2;
		//front_edge1 = front_pos4 - front_pos2;
		//front_edge2 = front_pos4 - front_pos1;
		//front_deltaUV1 = front_uv4 - front_uv2;
		//front_deltaUV2 = front_uv4 - front_uv1;
		//CalculateTangentBitangent(front_tangent2, 
		//						  front_bitangent2, 
		//						  front_edge1, 
		//						  front_edge2, 
		//						  front_deltaUV1, 
		//						  front_deltaUV2);
		//// Left cube face
		//glm::vec3 left_pos1(-0.5, -0.5, -0.5);
		//glm::vec3 left_pos2(-0.5,  0.5,  0.5);
		//glm::vec3 left_pos3(-0.5,  0.5, -0.5);
		//glm::vec3 left_pos4(-0.5, -0.5,  0.5);
		//glm::vec2 left_uv1(0.0, 0.0);
		//glm::vec2 left_uv2(1.0, 1.0);
		//glm::vec2 left_uv3(0.0, 1.0);
		//glm::vec2 left_uv4(1.0, 0.0);
		//glm::vec3 left_nm(-1.0, 0.0, 0.0);
		//// Calculate tangent/bitangent vectors of both left facing triangles
		//// Triangle 1
		//glm::vec3 left_tangent1;
		//glm::vec3 left_bitangent1;
		//glm::vec3 left_edge1 =    left_pos2 - left_pos1;
		//glm::vec3 left_edge2 =    left_pos3 - left_pos1;
		//glm::vec2 left_deltaUV1 = left_uv2 -  left_uv1;
		//glm::vec2 left_deltaUV2 = left_uv3 -  left_uv1;
		//CalculateTangentBitangent(left_tangent1, 
		//						  left_bitangent1, 
		//						  left_edge1, 
		//						  left_edge2, 
		//						  left_deltaUV1, 
		//						  left_deltaUV2);
		//// Triangle 2
		//glm::vec3 left_tangent2;
		//glm::vec3 left_bitangent2;
		//left_edge1 =    left_pos4 - left_pos2;
		//left_edge2 =    left_pos4 - left_pos1;
		//left_deltaUV1 = left_uv4 -  left_uv2;
		//left_deltaUV2 = left_uv4 -  left_uv1;
		//CalculateTangentBitangent(left_tangent2, 
		//						  left_bitangent2, 
		//						  left_edge1, 
		//						  left_edge2, 
		//						  left_deltaUV1, 
		//						  left_deltaUV2);
		//// Right cube face
		//glm::vec3 right_pos1(0.5, -0.5,  0.5);
		//glm::vec3 right_pos2(0.5,  0.5, -0.5);
		//glm::vec3 right_pos3(0.5,  0.5,  0.5);
		//glm::vec3 right_pos4(0.5, -0.5, -0.5);
		//glm::vec2 right_uv1(0.0, 0.0);
		//glm::vec2 right_uv2(1.0, 1.0);
		//glm::vec2 right_uv3(0.0, 1.0);
		//glm::vec2 right_uv4(1.0, 0.0);
		//glm::vec3 right_nm(1.0, 0.0, 0.0);
		//// Calculate tangent/bitangent vectors of both right facing triangles
		//// Triangle 1
		//glm::vec3 right_tangent1;
		//glm::vec3 right_bitangent1;
		//glm::vec3 right_edge1 =    right_pos2 - right_pos1;
		//glm::vec3 right_edge2 =    right_pos3 - right_pos1;
		//glm::vec2 right_deltaUV1 = right_uv2 -  right_uv1;
		//glm::vec2 right_deltaUV2 = right_uv3 -  right_uv1;
		//CalculateTangentBitangent(right_tangent1, 
		//						  right_bitangent1, 
		//						  right_edge1, 
		//						  right_edge2, 
		//						  right_deltaUV1, 
		//						  right_deltaUV2);
		//// Triangle 2
		//glm::vec3 right_tangent2;
		//glm::vec3 right_bitangent2;
		//right_edge1 =    right_pos4 - right_pos2;
		//right_edge2 =    right_pos4 - right_pos1;
		//right_deltaUV1 = right_uv4 -  right_uv2;
		//right_deltaUV2 = right_uv4 -  right_uv1;
		//CalculateTangentBitangent(right_tangent2, 
		//						  right_bitangent2, 
		//						  right_edge1, 
		//						  right_edge2, 
		//						  right_deltaUV1, 
		//						  right_deltaUV2);
		//// Top cube face
		//glm::vec3 top_pos1( 0.5,  0.5,  0.5);
		//glm::vec3 top_pos2(-0.5,  0.5, -0.5);
		//glm::vec3 top_pos3(-0.5,  0.5,  0.5);
		//glm::vec3 top_pos4( 0.5,  0.5, -0.5);
		//glm::vec2 top_uv1(0.0, 0.0);
		//glm::vec2 top_uv2(1.0, 1.0);
		//glm::vec2 top_uv3(1.0, 0.0);
		//glm::vec2 top_uv4(0.0, 1.0);
		//glm::vec3 top_nm(0.0, 1.0, 0.0);
		//// Calculate tangent/bitangent vectors of both top facing triangles
		//// Triangle 1
		//glm::vec3 top_tangent1;
		//glm::vec3 top_bitangent1;
		//glm::vec3 top_edge1 =    top_pos2 - top_pos1;
		//glm::vec3 top_edge2 =    top_pos3 - top_pos1;
		//glm::vec2 top_deltaUV1 = top_uv2 -  top_uv1;
		//glm::vec2 top_deltaUV2 = top_uv3 -  top_uv1;
		//CalculateTangentBitangent(top_tangent1, 
		//						  top_bitangent1, 
		//						  top_edge1, 
		//						  top_edge2, 
		//						  top_deltaUV1, 
		//						  top_deltaUV2);
		//// Triangle 2
		//glm::vec3 top_tangent2;
		//glm::vec3 top_bitangent2;
		//top_edge1 =    top_pos4 - top_pos2;
		//top_edge2 =    top_pos4 - top_pos1;
		//top_deltaUV1 = top_uv4 -  top_uv2;
		//top_deltaUV2 = top_uv4 -  top_uv1;
		//CalculateTangentBitangent(top_tangent2, 
		//						  top_bitangent2, 
		//						  top_edge1, 
		//						  top_edge2, 
		//						  top_deltaUV1, 
		//						  top_deltaUV2);
		//// Bottom cube face
		//glm::vec3 bottom_pos1(-0.5, -0.5,  0.5);
		//glm::vec3 bottom_pos2( 0.5, -0.5, -0.5);
		//glm::vec3 bottom_pos3( 0.5, -0.5,  0.5);
		//glm::vec3 bottom_pos4(-0.5, -0.5, -0.5);
		//glm::vec2 bottom_uv1(0.0, 0.0);
		//glm::vec2 bottom_uv2(1.0, 1.0);
		//glm::vec2 bottom_uv3(1.0, 0.0);
		//glm::vec2 bottom_uv4(0.0, 1.0);
		//glm::vec3 bottom_nm(0.0, -1.0, 0.0);
		//// Calculate tangent/bitangent vectors of both bottom facing triangles
		//// Triangle 1
		//glm::vec3 bottom_tangent1;
		//glm::vec3 bottom_bitangent1;
		//glm::vec3 bottom_edge1 =    bottom_pos2 - bottom_pos1;
		//glm::vec3 bottom_edge2 =    bottom_pos3 - bottom_pos1;
		//glm::vec2 bottom_deltaUV1 = bottom_uv2 -  bottom_uv1;
		//glm::vec2 bottom_deltaUV2 = bottom_uv3 -  bottom_uv1;
		//CalculateTangentBitangent(bottom_tangent1, 
		//						  bottom_bitangent1, 
		//						  bottom_edge1, 
		//						  bottom_edge2, 
		//						  bottom_deltaUV1, 
		//						  bottom_deltaUV2);
		//// Triangle 2
		//glm::vec3 bottom_tangent2;
		//glm::vec3 bottom_bitangent2;
		//bottom_edge1 = bottom_pos4 - bottom_pos2;
		//bottom_edge2 = bottom_pos4 - bottom_pos1;
		//bottom_deltaUV1 = bottom_uv4 - bottom_uv2;
		//bottom_deltaUV2 = bottom_uv4 -  bottom_uv1;
		//CalculateTangentBitangent(bottom_tangent2, 
		//						  bottom_bitangent2, 
		//						  bottom_edge1, 
		//						  bottom_edge2, 
		//						  bottom_deltaUV1, 
		//						  bottom_deltaUV2);
		
		float cubeVertices[] = {
			// positions      --  tex coords   --    normals
			   0.5,  0.5, -0.5,    1.0, 1.0,      0.0, 0.0, -1.0, // Cube back
			  -0.5, -0.5, -0.5,    0.0, 0.0,      0.0, 0.0, -1.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,      0.0, 0.0, -1.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,      0.0, 0.0, -1.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0,      0.0, 0.0, 1.0, // Cube front
			   0.5,  0.5,  0.5,    1.0, 1.0,      0.0, 0.0, 1.0,
			  -0.5,  0.5,  0.5,    0.0, 1.0,      0.0, 0.0, 1.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,      0.0, 0.0, 1.0,

			  -0.5, -0.5, -0.5,    0.0, 0.0,      -1.0, 0.0, 0.0, // Cube left
			  -0.5,  0.5,  0.5,    1.0, 1.0,      -1.0, 0.0, 0.0,
			  -0.5,  0.5, -0.5,    0.0, 1.0,      -1.0, 0.0, 0.0,
			  -0.5, -0.5,  0.5,    1.0, 0.0,      -1.0, 0.0, 0.0,

			   0.5, -0.5,  0.5,    0.0, 0.0,      1.0, 0.0, 0.0, // Cube right
			   0.5,  0.5, -0.5,    1.0, 1.0,      1.0, 0.0, 0.0,
			   0.5,  0.5,  0.5,    0.0, 1.0,      1.0, 0.0, 0.0,
			   0.5, -0.5, -0.5,    1.0, 0.0,      1.0, 0.0, 0.0,

			   0.5,  0.5,  0.5,    0.0, 0.0,      0.0, 1.0, 0.0, // Cube top
			  -0.5,  0.5, -0.5,    1.0, 1.0,      0.0, 1.0, 0.0,
			  -0.5,  0.5,  0.5,    1.0, 0.0,      0.0, 1.0, 0.0,
			   0.5,  0.5, -0.5,    0.0, 1.0,      0.0, 1.0, 0.0,

			  -0.5, -0.5,  0.5,    0.0, 0.0,      0.0, -1.0, 0.0, // Cube bottom
			   0.5, -0.5, -0.5,    1.0, 1.0,      0.0, -1.0, 0.0,
			   0.5, -0.5,  0.5,    1.0, 0.0,      0.0, -1.0, 0.0,
			  -0.5, -0.5, -0.5,    0.0, 1.0,      0.0, -1.0, 0.0,
		};
		//float cubeVertices[] = {
			// positions                              // normal                           // texcoords              // tangent/bitangent                                         
			/*back_pos1.x, back_pos1.y, back_pos1.z, back_nm.x, back_nm.y, back_nm.z, back_uv1.x, back_uv1.y, back_tangent1.x, back_tangent1.y, back_tangent1.z, back_bitangent1.x, back_bitangent1.y, back_bitangent1.z,
			back_pos2.x, back_pos2.y, back_pos2.z, back_nm.x, back_nm.y, back_nm.z, back_uv2.x, back_uv2.y, back_tangent1.x, back_tangent1.y, back_tangent1.z, back_bitangent1.x, back_bitangent1.y, back_bitangent1.z,
			back_pos3.x, back_pos3.y, back_pos3.z, back_nm.x, back_nm.y, back_nm.z, back_uv3.x, back_uv3.y, back_tangent1.x, back_tangent1.y, back_tangent1.z, back_bitangent1.x, back_bitangent1.y, back_bitangent1.z,
			back_pos4.x, back_pos4.y, back_pos4.z, back_nm.x, back_nm.y, back_nm.z, back_uv4.x, back_uv4.y, back_tangent1.x, back_tangent1.y, back_tangent1.z, back_bitangent1.x, back_bitangent1.y, back_bitangent1.z,

			front_pos1.x, front_pos1.y, front_pos1.z, front_nm.x, front_nm.y, front_nm.z, front_uv1.x, front_uv1.y, front_tangent1.x, front_tangent1.y, front_tangent1.z, front_bitangent1.x, front_bitangent1.y, front_bitangent1.z,
			front_pos2.x, front_pos2.y, front_pos2.z, front_nm.x, front_nm.y, front_nm.z, front_uv2.x, front_uv2.y, front_tangent1.x, front_tangent1.y, front_tangent1.z, front_bitangent1.x, front_bitangent1.y, front_bitangent1.z,
			front_pos3.x, front_pos3.y, front_pos3.z, front_nm.x, front_nm.y, front_nm.z, front_uv3.x, front_uv3.y, front_tangent1.x, front_tangent1.y, front_tangent1.z, front_bitangent1.x, front_bitangent1.y, front_bitangent1.z,
			front_pos4.x, front_pos4.y, front_pos4.z, front_nm.x, front_nm.y, front_nm.z, front_uv4.x, front_uv4.y, front_tangent1.x, front_tangent1.y, front_tangent1.z, front_bitangent1.x, front_bitangent1.y, front_bitangent1.z,

			left_pos1.x, left_pos1.y, left_pos1.z, left_nm.x, left_nm.y, left_nm.z, left_uv1.x, left_uv1.y, left_tangent1.x, left_tangent1.y, left_tangent1.z, left_bitangent1.x, left_bitangent1.y, left_bitangent1.z,
			left_pos2.x, left_pos2.y, left_pos2.z, left_nm.x, left_nm.y, left_nm.z, left_uv2.x, left_uv2.y, left_tangent1.x, left_tangent1.y, left_tangent1.z, left_bitangent1.x, left_bitangent1.y, left_bitangent1.z,
			left_pos3.x, left_pos3.y, left_pos3.z, left_nm.x, left_nm.y, left_nm.z, left_uv3.x, left_uv3.y, left_tangent1.x, left_tangent1.y, left_tangent1.z, left_bitangent1.x, left_bitangent1.y, left_bitangent1.z,
			left_pos4.x, left_pos4.y, left_pos4.z, left_nm.x, left_nm.y, left_nm.z, left_uv4.x, left_uv4.y, left_tangent1.x, left_tangent1.y, left_tangent1.z, left_bitangent1.x, left_bitangent1.y, left_bitangent1.z,

			right_pos1.x, right_pos1.y, right_pos1.z, right_nm.x, right_nm.y, right_nm.z, right_uv1.x, right_uv1.y, right_tangent1.x, right_tangent1.y, right_tangent1.z, right_bitangent1.x, right_bitangent1.y, right_bitangent1.z,
			right_pos2.x, right_pos2.y, right_pos2.z, right_nm.x, right_nm.y, right_nm.z, right_uv2.x, right_uv2.y, right_tangent1.x, right_tangent1.y, right_tangent1.z, right_bitangent1.x, right_bitangent1.y, right_bitangent1.z,
			right_pos3.x, right_pos3.y, right_pos3.z, right_nm.x, right_nm.y, right_nm.z, right_uv3.x, right_uv3.y, right_tangent1.x, right_tangent1.y, right_tangent1.z, right_bitangent1.x, right_bitangent1.y, right_bitangent1.z,
			right_pos4.x, right_pos4.y, right_pos4.z, right_nm.x, right_nm.y, right_nm.z, right_uv4.x, right_uv4.y, right_tangent1.x, right_tangent1.y, right_tangent1.z, right_bitangent1.x, right_bitangent1.y, right_bitangent1.z,

			top_pos1.x, top_pos1.y, top_pos1.z, top_nm.x, top_nm.y, top_nm.z, top_uv1.x, top_uv1.y, top_tangent1.x, top_tangent1.y, top_tangent1.z, top_bitangent1.x, top_bitangent1.y, top_bitangent1.z,
			top_pos2.x, top_pos2.y, top_pos2.z, top_nm.x, top_nm.y, top_nm.z, top_uv2.x, top_uv2.y, top_tangent1.x, top_tangent1.y, top_tangent1.z, top_bitangent1.x, top_bitangent1.y, top_bitangent1.z,
			top_pos3.x, top_pos3.y, top_pos3.z, top_nm.x, top_nm.y, top_nm.z, top_uv3.x, top_uv3.y, top_tangent1.x, top_tangent1.y, top_tangent1.z, top_bitangent1.x, top_bitangent1.y, top_bitangent1.z,
			top_pos4.x, top_pos4.y, top_pos4.z, top_nm.x, top_nm.y, top_nm.z, top_uv4.x, top_uv4.y, top_tangent1.x, top_tangent1.y, top_tangent1.z, top_bitangent1.x, top_bitangent1.y, top_bitangent1.z,

			bottom_pos1.x, bottom_pos1.y, bottom_pos1.z, bottom_nm.x, bottom_nm.y, bottom_nm.z, bottom_uv1.x, bottom_uv1.y, bottom_tangent1.x, bottom_tangent1.y, bottom_tangent1.z, bottom_bitangent1.x, bottom_bitangent1.y, bottom_bitangent1.z,
			bottom_pos2.x, bottom_pos2.y, bottom_pos2.z, bottom_nm.x, bottom_nm.y, bottom_nm.z, bottom_uv2.x, bottom_uv2.y, bottom_tangent1.x, bottom_tangent1.y, bottom_tangent1.z, bottom_bitangent1.x, bottom_bitangent1.y, bottom_bitangent1.z,
			bottom_pos3.x, bottom_pos3.y, bottom_pos3.z, bottom_nm.x, bottom_nm.y, bottom_nm.z, bottom_uv3.x, bottom_uv3.y, bottom_tangent1.x, bottom_tangent1.y, bottom_tangent1.z, bottom_bitangent1.x, bottom_bitangent1.y, bottom_bitangent1.z,
			bottom_pos4.x, bottom_pos4.y, bottom_pos4.z, bottom_nm.x, bottom_nm.y, bottom_nm.z, bottom_uv4.x, bottom_uv4.y, bottom_tangent1.x, bottom_tangent1.y, bottom_tangent1.z, bottom_bitangent1.x, bottom_bitangent1.y, bottom_bitangent1.z*/
		//};

		unsigned int cubeIndices[]{
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
		m_VA_Ground->Bind();
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
		m_VA_Cube->Bind();
		// Init Vertex Buffer and bind to Vertex Array 
		m_VB_Cube = new VertexBuffer(cubeVertices, 8 * 4 * 6 * sizeof(float));
		// Create and associate the layout (Vertex Attribute Pointer)
		VertexBufferLayout cubeVBLayout;
		cubeVBLayout.Push<float>(3); // Vertex position,	 vec3
		cubeVBLayout.Push<float>(3); // Normals,			 vec3
		cubeVBLayout.Push<float>(2); // Texture coordinates, vec2
		//cubeVBLayout.Push<float>(3); // Tangents,			 vec3
		//cubeVBLayout.Push<float>(3); // Bitangents,	     vec3
		m_VA_Cube->AddBuffer(*m_VB_Cube, cubeVBLayout);
		// Init index buffer and bind to Vertex Array 
		m_IB_Cube = new IndexBuffer(cubeIndices, 6 * 6);

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	TestPointShadowMapping::~TestPointShadowMapping()
	{
	}

	void TestPointShadowMapping::OnUpdate(float deltaTime)
	{
	}

	void TestPointShadowMapping::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 4.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Calculate deltaTime
		float currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		// Process keyboard inputs
		processInputPointShadowMapping(m_MainWindow);

		Renderer renderer;

		// Flashlight position and direction
		glm::vec3 flashlightPosition = m_Camera.Position;
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0), 2.0f * m_Camera.Right);
		flashlightPosition = glm::vec3(translationMatrix * glm::vec4(flashlightPosition, 1.0));
		glm::vec3 flashlightDirection = m_Camera.Front;
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(4.0f), glm::vec3(0.0, 1.0, 0.0));
		flashlightDirection = glm::vec3(rotationMatrix * glm::vec4(flashlightDirection, 1.0));

		// First render to the flashlight's (perspective) shadow depth map 
		//
		// Bind shadow map framebuffer
		glViewport(0, 0, m_ShadowMapWidth, m_ShadowMapHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		// Cull front faces while filling the shadow depth buffer to avoid Peter Panning of shadows
		glCullFace(GL_FRONT); // TODO not necessary for perspective shadow mapping?
		// Configure matrices and shader
		m_ShadowDepthMapShader->Bind();
		float near_plane = 0.1f, far_plane = 400.0f;
		glm::mat4 lightModelMatrix1 = glm::mat4(1.0f);
		float movementAmount = 8.0f;
		float cube1PositionX = 0.0f; // movementAmount* sin(glfwGetTime() / 2.0f);
		float cube1PositionY = 0.0f; // movementAmount * cos(glfwGetTime() / 1.0f);
		float cube1PositionZ = 0.0f;
		glm::vec3 cube1Position = glm::vec3(cube1PositionX, cube1PositionY, cube1PositionZ);
		lightModelMatrix1 = glm::translate(lightModelMatrix1, cube1Position);
		lightModelMatrix1 = glm::scale(lightModelMatrix1, glm::vec3(4.0));
		glm::mat4 lightViewMatrixPerspective = glm::lookAt(flashlightPosition,
															flashlightDirection,		
															m_Camera.Up);	
		glm::mat4 lightProjectionMatrixPerspective = glm::perspective(glm::radians(135.0f), 1.0f, 0.1f, 1000.0f);
		glm::mat4 lightSpaceMatrixPerspective = lightProjectionMatrixPerspective * lightViewMatrixPerspective;
		m_ShadowDepthMapShader->SetMatrix4f("lightSpaceMatrix", lightSpaceMatrixPerspective);
		m_ShadowDepthMapShader->SetMatrix4f("lightModel", lightModelMatrix1);
		// Create depth map from flashlight's POV by rendering the scene
		// Draw first cube
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_ShadowDepthMapShader);
		// Change model matrix and draw second cube
		float cube2PositionX = 4.0f;
		float cube2PositionY = 4.0f; //movementAmount * sin(glfwGetTime() / 2.0f);
		float cube2PositionZ = 4.0f; //movementAmount * cos(glfwGetTime() / 1.0f);
		glm::vec3 cube2Position = glm::vec3(cube2PositionX, cube2PositionY, cube2PositionZ);
		glm::mat4 lightModelMatrix2 = glm::mat4(1.0f);
		lightModelMatrix2 = glm::translate(lightModelMatrix2, cube2Position);
		lightModelMatrix2 = glm::scale(lightModelMatrix2, glm::vec3(4.0));
		m_ShadowDepthMapShader->SetMatrix4f("lightModel", lightModelMatrix2);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_ShadowDepthMapShader);
		// Change model matrix and draw second cube
		float cube3PositionX = movementAmount * sin(glfwGetTime() / 10.0f);
		float cube3PositionY = 0.0f;
		float cube3PositionZ = 1.1f * movementAmount * cos(glfwGetTime() / 5.0f);
		glm::vec3 cube3Position = glm::vec3(cube3PositionX, cube3PositionY, cube3PositionZ);
		glm::mat4 lightModelMatrix3 = glm::mat4(1.0f);
		lightModelMatrix3 = glm::translate(lightModelMatrix3, cube3Position);
		lightModelMatrix3 = glm::rotate(lightModelMatrix3, (float)sin(glfwGetTime() / 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		lightModelMatrix3 = glm::scale(lightModelMatrix3, glm::vec3(4.0));
		m_ShadowDepthMapShader->SetMatrix4f("lightModel", lightModelMatrix3);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_ShadowDepthMapShader);
		// Then draw the ground
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_ShadowDepthMapShader);

		// Then pass the perspective shadow depth map to the other shader
		m_Shader->Bind();
		// Bind shadow depth map texture to shader
		GLCall(glActiveTexture(GL_TEXTURE3));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_ShadowDepthMap));
		m_Shader->SetInt("shadowMapPerspective", 3);

		// Then return to the default framebuffer and render the scene as normal, using the depth map to create shadows
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Return to culling back faces
		glCullFace(GL_BACK);

		// Set per-frame uniforms
		m_Shader->Bind();
		m_Shader->SetBool("u_UsingOrthographicShadowMapping", false);
		m_Shader->SetBool("u_UsingPerspectiveShadowMapping", true);
		// Create model, view, projection matrices
		glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
		glm::mat4 projMatrix = glm::perspective(glm::radians(m_Camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 800.0f);
		m_Shader->SetMatrix4f("model", lightModelMatrix1);
		m_Shader->SetMatrix4f("view", viewMatrix);
		m_Shader->SetMatrix4f("proj", projMatrix);
		// Update camera's viewing position each frame
		m_Shader->SetVec3f("viewPos", m_Camera.Position.x, m_Camera.Position.y, m_Camera.Position.z);
		// Set lightSpaceMatrix in this shader
		m_Shader->SetMatrix4f("lightSpaceMatrixPerspective", lightSpaceMatrixPerspective);

		// Flashlight's properties
		//
		m_Shader->SetBool("u_Flashlight.on", m_IsFlashlightOn);
		m_Shader->SetVec3("u_Flashlight.ambient", m_FlashlightAmbientColour);
		m_Shader->SetVec3("u_Flashlight.diffuse", m_FlashlightDiffuseColour);
		m_Shader->SetVec3("u_Flashlight.specular", m_FlashlightSpecularIntensity);
		// Flashlight attenuation properties
		m_Shader->SetFloat("u_Flashlight.constant", 1.0f);
		m_Shader->SetFloat("u_Flashlight.linear", 0.02f);
		m_Shader->SetFloat("u_Flashlight.quadratic", 0.01f);
		// Flashlight position and direction
		m_Shader->SetVec3f("u_Flashlight.position", flashlightPosition.x, flashlightPosition.y, flashlightPosition.z);
		m_Shader->SetVec3f("u_Flashlight.direction", flashlightDirection.x, flashlightDirection.y, flashlightDirection.z);
		// Flashlight cutoff angle
		m_Shader->SetFloat("u_Flashlight.cutOff", glm::cos(glm::radians(20.0f)));
		m_Shader->SetFloat("u_Flashlight.outerCutOff", glm::cos(glm::radians(50.0f)));

		// Render the first cube
		m_ContainerTexture->Bind(1);
		m_Shader->SetInt("u_Material.diffuse", 1);
		//m_Shader->SetBool("u_UsingNormalMap", m_UsingNormalMap);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Change model matrix and render the second cube
		m_Shader->SetMatrix4f("model", lightModelMatrix2);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Change model matrix and render the third cube
		m_Shader->SetMatrix4f("model", lightModelMatrix3);
		m_BrickTexture->Bind(1);
		m_Shader->SetInt("u_Material.diffuse", 1);
		//m_Shader->SetBool("u_UsingNormalMap", false);
		//m_BrickNormalMap->Bind(4);
		//m_Shader->SetInt("normalMap", 4);
		renderer.DrawTriangles(*m_VA_Cube, *m_IB_Cube, *m_Shader);

		// Set any different uniforms for the ground
		glm::mat4 modelMatrix = glm::mat4(1.0);
		m_Shader->SetMatrix4f("model", modelMatrix);
		// Render the ground
		m_GroundTexture->BindAndSetRepeating(0);
		m_Shader->SetInt("u_Material.diffuse", 0);
		renderer.DrawTriangles(*m_VA_Ground, *m_IB_Ground, *m_Shader);
	}

	void TestPointShadowMapping::OnImGuiRender()
	{
		// ImGui interface
		ImGui::Text("PRESS '3' to decrease shadow map resolution");
		ImGui::Text("PRESS '4' to increase shadow map resolution");
		ImGui::Text("PRESS '5' to turn off normal mapping");
		ImGui::Text("PRESS '6' to turn on normal mapping");
		ImGui::Text(" - - - ");
		ImGui::Text("PRESS 'BACKSPACE' TO EXIT");
		ImGui::Text("- Use WASD keys to move camera");
		ImGui::Text("- Use scroll wheel to change FOV");
		ImGui::Text("- Press '1' and '2' to toggle wireframe mode");
		ImGui::Text("- Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	void TestPointShadowMapping::OnActivated()
	{
		// Hide and capture mouse cursor
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		//  Reset all uniforms
		m_Shader->Bind();
		m_Shader->SetVec3f("u_Material.specular", 0.1f, 0.1f, 0.1f);
		m_Shader->SetFloat("u_Material.shininess", 4.0f);

		// Directional light
		m_Shader->SetVec3("u_DirLight.direction", m_DirLightDirection);
		m_Shader->SetVec3("u_DirLight.ambient", m_DirLightAmbient);
		m_Shader->SetVec3("u_DirLight.diffuse", m_DirLightDiffuse);
		m_Shader->SetVec3("u_DirLight.specular", m_DirLightSpecular); 

		// Generate framebuffer for shadow mapping
		glGenFramebuffers(1, &m_DepthMapFBO);
		// 2D buffer for storing depth values
		glGenTextures(1, &m_ShadowDepthMap);
		glBindTexture(GL_TEXTURE_2D, m_ShadowDepthMap);
		// We only need the depth information when rendering the scene from the light's perspective, so no need for a colour or stencil buffer
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_ShadowMapWidth, m_ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// Attach the shadow map buffer to the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_ShadowDepthMap, 0);
		//A framebuffer is not 'complete' without a colour buffer so we need to explicitly set OpenGL to not render any colour data
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		// Reset to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, mouse_callbackPointShadowMapping);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, scroll_callbackPointShadowMapping);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, mouse_button_callbackPointShadowMapping);
	}

	void TestPointShadowMapping::ShadowResolution(const int dir)
	{
		if (dir == -1)
		{
			// Lower resolution of shadow map (down to a minimum)
			m_ShadowMapWidth *= 0.90f;
			m_ShadowMapHeight *= 0.90f;
			if (m_ShadowMapWidth < 10) m_ShadowMapWidth = 10;
			if (m_ShadowMapHeight < 10) m_ShadowMapHeight = 10;
			OnActivated();
		}
		else if (dir == 1)
		{
			// Increase resolution of shadow map (up to a maximum)
			m_ShadowMapWidth *= 1.1f;
			m_ShadowMapHeight *= 1.1f;
			if (m_ShadowMapWidth > 2048) m_ShadowMapWidth = 2048;
			if (m_ShadowMapHeight > 2048) m_ShadowMapHeight = 2048;
			OnActivated();
		}
	}

	void TestPointShadowMapping::ToggleNormalMapping(bool flag)
	{
		m_UsingNormalMap = flag;
	}

	void scroll_callbackPointShadowMapping(GLFWwindow* window, double xOffset, double yOffset)
	{
		test::TestPointShadowMapping* shadowMapping = test::TestPointShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();
		shadowMappingCamera->ProcessMouseScroll(yOffset);
	}

	void mouse_callbackPointShadowMapping(GLFWwindow* window, double xpos, double ypos)
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

		test::TestPointShadowMapping* shadowMapping = test::TestPointShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();
		shadowMappingCamera->ProcessMouseMovement(xOffset, yOffset);
	}

	void mouse_button_callbackPointShadowMapping(GLFWwindow* window, int button, int action, int mods)
	{
		// test::TestPointShadowMapping* shadowMapping = test::TestPointShadowMapping::GetInstance();

		//if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		//    ...
	}

	void processInputPointShadowMapping(GLFWwindow* window) {
		test::TestPointShadowMapping* shadowMapping = test::TestPointShadowMapping::GetInstance();
		Camera* shadowMappingCamera = shadowMapping->GetCamera();

		// Camera position movement
		float cameraYPosition = shadowMappingCamera->Position.y;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(FORWARD,  deltaTime);
			//shadowMappingCamera->ProcessKeyboardForWalkingView(FORWARD,  deltaTime, cameraYPosition);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(LEFT,	 deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			shadowMappingCamera->ProcessKeyboard(RIGHT,	 deltaTime);
		// Shadow res
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			shadowMapping->ShadowResolution(-1);
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			shadowMapping->ShadowResolution(1);
		// Toggle normal mapping
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			shadowMapping->ToggleNormalMapping(false);
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			shadowMapping->ToggleNormalMapping(true);
	}
}

