#include "GraphicsInstance.h"
#include <Runtime/Core/Core.h>

namespace Oksijen
{
#ifdef OKSIJEN_DEBUG
	PFN_vkCreateDebugUtilsMessengerEXT debugMessengerCreateProc = NULL;
	PFN_vkDestroyDebugUtilsMessengerEXT debugMessengerDestroyProc = NULL;
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		DEV_LOG("VulkanDebug", "%s", pCallbackData->pMessage);
		return VK_FALSE;
	}
#endif
	GraphicsInstance::GraphicsInstance(const std::string& appName, const std::string& engineName, const unsigned int appVersion, const unsigned int apiVersion, const std::vector<std::string>& extensions, const std::vector<std::string>& layers)
	{
		//Get requested extensions
		std::vector<std::string> requestedExtensions;
		for (const std::string& extension : extensions)
			requestedExtensions.push_back(extension);

#ifdef OKSIJEN_DEBUG
		requestedExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		requestedExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		
		//Get supported extension count
		unsigned int extensionCount = 0;
		DEV_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) == VK_SUCCESS, "GraphicsInstance", "Failed to get instance extension count");
		DEV_ASSERT(extensionCount > 0, "GraphicsInstance", "No extensions found");

		//Get supported extensions
		std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
		DEV_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data()) == VK_SUCCESS, "GraphicsInstance", "Failed to get the supported extensions");

		//Validate extensions
		std::vector<const char*> validatedExtensions;
		for (unsigned int instanceExtensionIndex = 0; instanceExtensionIndex < requestedExtensions.size(); instanceExtensionIndex++)
		{
			const std::string& entry = requestedExtensions[instanceExtensionIndex];

			//Check if properties contains this extension
			bool bExtensionSupported = false;
			for (unsigned int supportedInstanceExtensionIndex = 0; supportedInstanceExtensionIndex < extensionCount; supportedInstanceExtensionIndex++)
			{
				const VkExtensionProperties& supportedInstanceExtensionProperty = supportedExtensions[supportedInstanceExtensionIndex];

				if (strcmp(entry.c_str(), supportedInstanceExtensionProperty.extensionName) == 0)
				{
					validatedExtensions.push_back(entry.c_str());
					break;
				}
			}
		}

		DEV_ASSERT(validatedExtensions.size() <= requestedExtensions.size(), "GraphicsInstance", "Some of the requested extensions are not supported!");

		//Get requested layers
		std::vector<std::string> requestedLayers;
		for (const std::string& layer : layers)
			requestedExtensions.push_back(layer);

#ifdef OKSIJEN_DEBUG
		requestedLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

		//Get supported layer count
		unsigned int layerCount = 0;
		DEV_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, nullptr) == VK_SUCCESS, "GraphicsInstance", "Failed to get intance layer count");
		DEV_ASSERT(layerCount > 0, "GraphicsInstance", "No layers found");

		std::vector<VkLayerProperties> supportedLayers(layerCount);
		DEV_ASSERT(vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data()) == VK_SUCCESS, "GraphicsInstance", "Failed to get the supported layers");

		//Validate layers
		std::vector<const char*> validatedLayers;
		for (const std::string& layerString : requestedLayers)
		{
			for (const VkLayerProperties& layer : supportedLayers)
			{
				if (strcmp(layer.layerName, layerString.c_str()) == 0)
				{
					validatedLayers.push_back(layerString.c_str());
					break;
				}
			}
		}
		DEV_ASSERT(validatedLayers.size() <= requestedLayers.size(), "GraphicsInstance", "Some of the requested layers are not supported!");

		//Create instance
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = appVersion;
		appInfo.pEngineName = engineName.c_str();
		appInfo.engineVersion = 0;
		appInfo.apiVersion = apiVersion;

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.flags = VkInstanceCreateFlags();
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = validatedExtensions.size();
		instanceInfo.ppEnabledExtensionNames = validatedExtensions.data();
		instanceInfo.enabledLayerCount = validatedLayers.size();
		instanceInfo.ppEnabledLayerNames = validatedLayers.data();
		instanceInfo.pNext = nullptr;

		DEV_ASSERT(vkCreateInstance(&instanceInfo, nullptr, &mInstance) == VK_SUCCESS, "VulkanInstance", "Failed to create instance");

#ifdef OKSIJEN_DEBUG
		//Get debug proc addresses
		debugMessengerCreateProc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
		debugMessengerDestroyProc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");

		DEV_ASSERT(debugMessengerCreateProc != nullptr, "GraphicsInstance", "Failed to get vkCreateDebugUtilsMessengerEXT method!");
		DEV_ASSERT(debugMessengerDestroyProc != nullptr, "GraphicsInstance", "Failed to get vkDestroyDebugUtilsMessengerEXT method!");

		//Create debug messenger
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
		debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugMessengerInfo.pfnUserCallback = DebugCallBack;
		debugMessengerInfo.pUserData = nullptr;
		DEV_ASSERT(debugMessengerCreateProc(mInstance, &debugMessengerInfo, nullptr, &mDebugMessenger) == VK_SUCCESS, "GraphicsInstance", "Failed to create VkDebugUtilsMessengerEXT");
#endif

		//Get physical device count
		unsigned int physicalDeviceCount = 0;
		DEV_ASSERT(vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr) == VK_SUCCESS, "GraphicsInstance", "Failed to get the physical device count");
		DEV_ASSERT(physicalDeviceCount > 0, "GraphicsInstance", "No physical devices found!");

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		DEV_ASSERT(vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, physicalDevices.data()) == VK_SUCCESS, "GraphicsInstance", "Failed to get physical devices");

		for (const VkPhysicalDevice physicalDevice : physicalDevices)
		{
			mAdapters.push_back(new GraphicsAdapter(physicalDevice));
		}
	}
	GraphicsInstance::~GraphicsInstance()
	{
		vkDestroyInstance(mInstance, nullptr);
	}
	Surface* GraphicsInstance::CreateSurface(const PlatformWindow* pWindow)
	{
		return new Surface(pWindow,this);
	}
}