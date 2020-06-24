#include "TestModelLoading.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

namespace test
{
	TestModelLoading::TestModelLoading(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow)
	{
	}

	TestModelLoading::~TestModelLoading()
	{
	}

	void TestModelLoading::OnUpdate(float deltaTime)
	{
	}

	void TestModelLoading::OnRender()
	{
		float* clearColour = test::TestClearColour::GetClearColour();
		float darknessFactor = 1.0f;
		GLCall(glClearColor(clearColour[0] / darknessFactor, clearColour[1] / darknessFactor,
			clearColour[2] / darknessFactor, clearColour[3] / darknessFactor));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void TestModelLoading::OnImGuiRender()
	{
	}

	void TestModelLoading::OnActivated()
	{
	}
}

