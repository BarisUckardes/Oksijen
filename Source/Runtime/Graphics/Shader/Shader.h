#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>
#include <string>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API Shader final
	{
	public:
		Shader(const GraphicsDevice* pDevice,const std::string& entryPoint,const VkShaderStageFlags stage, const unsigned char* pBytes, const unsigned int byteCount);
		~Shader();

		FORCEINLINE VkShaderModule GetModule() const noexcept { return mModule; }
		FORCEINLINE const std::string& GetEntryPoint() const noexcept { return mEntryPoint; }
		FORCEINLINE VkShaderStageFlags GetStage() const noexcept { return mStage; }
	private:
		const VkDevice mLogicalDevice;
		const std::string mEntryPoint;
		const VkShaderStageFlags mStage;
		VkShaderModule mModule;
	};
}