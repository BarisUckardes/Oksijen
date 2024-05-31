#pragma once
#include <string>

namespace Oksijen
{
	struct RUNTIME_API WindowDesc
	{
		std::string Title;
		int X;
		int Y;
		unsigned int Width;
		unsigned int Height;
	};
}