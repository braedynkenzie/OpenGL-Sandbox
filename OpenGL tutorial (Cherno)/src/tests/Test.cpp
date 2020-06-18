#include "Test.h"

namespace test
{
	TestMenu::TestMenu(Test*& activeTestPtr)
		: m_CurrentTest(activeTestPtr)
	{
	}

	void TestMenu::OnImGuiRender()
	{
		for (auto& test : m_Tests)
		{
			if (ImGui::Button(test.first.c_str()))
			{
				m_CurrentTest = test.second;
			}
		}
	}
}


