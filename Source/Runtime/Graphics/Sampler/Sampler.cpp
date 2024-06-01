#include "Sampler.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{

	Sampler::Sampler(
		const GraphicsDevice* pDevice,
		const VkFilter magFilter, const VkFilter minFilter,
		const VkSamplerMipmapMode mipmapMode, const VkSamplerAddressMode u, const VkSamplerAddressMode v, const VkSamplerAddressMode w,
		const float mipLodBias,
		const VkBool32 bAnisotropyEnabled, const float maxAnisotropy,
		const VkBool32 bCompareEnabled, const VkCompareOp compareOp,
		const float minLod, const float maxLod,
		const VkBorderColor borderColor,
		const VkBool32 unnormalizedCoordinates) : mLogicalDevice(pDevice->GetLogicalDevice()), mSampler(VK_NULL_HANDLE)
	{
		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = magFilter;
		info.minFilter = minFilter;
		info.mipmapMode = mipmapMode;
		info.addressModeU = u;
		info.addressModeV = v;
		info.addressModeW = w;
		info.mipLodBias = mipLodBias;
		info.anisotropyEnable = bAnisotropyEnabled;
		info.maxAnisotropy = maxAnisotropy;
		info.compareEnable = bCompareEnabled;
		info.compareOp = compareOp;
		info.minLod = minLod;
		info.maxLod = maxLod;
		info.borderColor = borderColor;
		info.unnormalizedCoordinates = unnormalizedCoordinates;
		info.pNext = nullptr;

		DEV_ASSERT(vkCreateSampler(mLogicalDevice, &info, nullptr, &mSampler) == VK_SUCCESS, "Sampler", "Failed to create the sampler");
	}
	Sampler::~Sampler()
	{
		vkDestroySampler(mLogicalDevice, mSampler, nullptr);
	}
}