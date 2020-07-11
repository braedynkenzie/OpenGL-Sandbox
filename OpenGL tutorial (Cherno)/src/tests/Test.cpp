#include "Test.h"
#include <Globals.h>

namespace test
{
	// Init static variable
	TestMenu* TestMenu::instance;

	TestMenu::TestMenu(Test*& activeTestPtr, GLFWwindow* mainWindow)
		: m_CurrentTest(activeTestPtr),
		m_MainWindow(mainWindow)
	{
		instance = this;
	}

	void TestMenu::OnImGuiRender()
	{
		for (auto& testPair : m_Tests)
		{
			if (ImGui::Button(testPair.first.c_str()))
			{
				Test* newTest = testPair.second;
				m_CurrentTest = newTest;
				newTest->OnActivated();
			}
		}
	}

	void TestMenu::OnActivated()
	{
		// Do any updates that should be made when returning to main menu
		// Unhide and uncapture mouse cursor when switching between tests
		glfwSetInputMode(m_MainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		// Reset all callbacks
		// Callback function for mouse cursor movement
		glfwSetCursorPosCallback(m_MainWindow, nullptr);
		// Callback function for scrolling zoom
		glfwSetScrollCallback(m_MainWindow, nullptr);
		// Callback function for mouse buttons
		glfwSetMouseButtonCallback(m_MainWindow, nullptr);

		// Diable face culling
		GLCall(glDisable(GL_CULL_FACE));

		// Disable gl_PointSize in vertex shaders
		GLCall(glDisable(GL_PROGRAM_POINT_SIZE));
	}

	void TestMenu::SetScreenDimensions(unsigned int width, unsigned int height)
	{
		SCREEN_WIDTH = width;
		SCREEN_HEIGHT = height;
	}
}


