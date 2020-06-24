#include "TestTemplate.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

namespace test
{
	TestTemplate::TestTemplate(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow)
	{
	}

	TestTemplate::~TestTemplate()
	{
	}

	void TestTemplate::OnUpdate(float deltaTime)
	{
	}

	void TestTemplate::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void TestTemplate::OnImGuiRender()
	{
	}

	void TestTemplate::OnActivated()
	{
	}
}

