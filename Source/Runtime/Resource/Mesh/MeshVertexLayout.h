#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace Oksijen
{
	class RUNTIME_API MeshVertexLayout
	{
	public:
		MeshVertexLayout(const std::vector<VkFormat>& formats);
		~MeshVertexLayout() = default;

		FORCEINLINE const std::vector<VkFormat>& GetFormats() const noexcept { return mFormats; }
		FORCEINLINE unsigned int GetSize() const noexcept { return mSize; }
	private:
		const std::vector<VkFormat> mFormats;
		unsigned int mSize;
	};
}