#pragma once

#include "Test.h"

namespace test
{
	class TestClearColour : public Test
	{
	private:
		float m_ClearColour[4];

	public: 
		TestClearColour();
		~TestClearColour();

		void OnUpdate(float deltaTime);
		void OnRender();
		void OnImGuiRender();
	};
}
