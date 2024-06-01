#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API Sampler final
	{
	public:
		Sampler(
			const GraphicsDevice* pDevice,
			const VkFilter magFilter,const VkFilter minFilter,
			const VkSamplerMipmapMode mipmapMode,const VkSamplerAddressMode u,const VkSamplerAddressMode v,const VkSamplerAddressMode w,
			const float mipLodBias,
			const VkBool32 bAnisotropyEnabled,const float maxAnisotropy,
			const VkBool32 bCompareEnabled,const VkCompareOp compareOp,
			const float minLod,const float maxLod,
			const VkBorderColor borderColor,
			const VkBool32 unnormalizedCoordinates);
		~Sampler();

		FORCEINLINE VkSampler GetSampler() const noexcept { return mSampler; }
	private:
		const VkDevice mLogicalDevice;
		VkSampler mSampler;
	};
}