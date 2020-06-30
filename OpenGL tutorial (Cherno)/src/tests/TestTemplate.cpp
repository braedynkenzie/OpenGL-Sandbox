#include "TestTemplate.h"

#include "Renderer.h"
#include <tests\TestClearColour.h>

#include "Globals.h"

namespace test
{
	TestTemplate::TestTemplate(GLFWwindow*& mainWindow)
		: m_MainWindow(mainWindow)
	{

		// TODO fill in here with specifics

		// Enable OpenGL z-buffer depth comparisons
		glEnable(GL_DEPTH_TEST);
		// Render only those fragments with lower depth values
		glDepthFunc(GL_LESS);

		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
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
		// ImGui interface
		ImGui::Text("// TODO UI");
	}

	void TestTemplate::OnActivated()
	{
	}
}

