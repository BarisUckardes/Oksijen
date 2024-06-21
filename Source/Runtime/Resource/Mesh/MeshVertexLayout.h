#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Oksijen
{
	struct RUNTIME_API MeshVertexLayout
	{
		std::vector<VkFormat> Format;
	};
}