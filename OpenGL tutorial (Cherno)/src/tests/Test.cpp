#include "Test.h"

namespace test
{
	TestMenu::TestMenu(Test*& activeTestPtr, GLFWwindow* mainWindow)
		: m_CurrentTest(activeTestPtr),
		m_MainWindow(mainWindow)
	{
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
	}
}


