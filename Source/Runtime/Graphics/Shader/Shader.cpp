#include "Shader.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
    Shader::Shader(const GraphicsDevice* pDevice, const std::string& entryPoint, const VkShaderStageFlags stage, const unsigned char* pBytes, const unsigned int byteCount) 
        : mLogicalDevice(pDevice->GetLogicalDevice()),mModule(VK_NULL_HANDLE),mEntryPoint(entryPoint),mStage(stage)
    {
        VkShaderModuleCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.flags = VkShaderModuleCreateFlags();
        info.pCode = (const uint32_t*)pBytes;
        info.codeSize = byteCount;
        info.pNext = nullptr;

        DEV_ASSERT(vkCreateShaderModule(mLogicalDevice, &info,nullptr, &mModule) == VK_SUCCESS, "Shader", "Failed to create shader module");
    }
    Shader::~Shader()
    {
        vkDestroyShaderModule(mLogicalDevice, mModule, nullptr);
    }
}