#include "TestClearColour.h"

#include "Renderer.h"

namespace test
{
	float START_CLEAR_COLOUR[4] = { 0.2f, 0.8f, 0.8f, 1.0f };
	float TestClearColour::m_ClearColour[4] = { START_CLEAR_COLOUR[0], START_CLEAR_COLOUR[1], START_CLEAR_COLOUR[2], START_CLEAR_COLOUR[3] };

	TestClearColour::TestClearColour(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow)
	{
	}

	TestClearColour::~TestClearColour()
	{
	}

	void TestClearColour::OnUpdate(float deltaTime)
	{
	}

	void TestClearColour::OnRender()
	{
		GLCall(glClearColor(m_ClearColour[0], m_ClearColour[1], m_ClearColour[2], m_ClearColour[3]));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
	}

	void TestClearColour::OnImGuiRender()
	{
		ImGui::ColorEdit4("Clear colour", m_ClearColour);
	}

	void TestClearColour::OnActivated()
	{
	}
}

