#include "VulkanDevice.h"

#include <iostream>
#include <cassert>
#include <set>
#include "utils.h"

VulkanDevice::VulkanDevice()
{
	initialize();
}

VulkanDevice::~VulkanDevice()
{
	cleanUp();
}

void VulkanDevice::initialize()
{
	chk(SDL_Init(SDL_INIT_VIDEO));
	chk(SDL_Vulkan_LoadLibrary(NULL));

	volkInitialize();

	// INSTANCE
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Path Tracer";
	appInfo.apiVersion = VK_API_VERSION_1_4;

	uint32_t instanceCount{ 0 };
	char const* const* instanceExtensions{ SDL_Vulkan_GetInstanceExtensions(&instanceCount) };

	VkInstanceCreateInfo instanceCI{};
	instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCI.pApplicationInfo = &appInfo;
	instanceCI.enabledExtensionCount = instanceCount;
	instanceCI.ppEnabledExtensionNames = instanceExtensions;
	// validation layer
	if (enableValidationLayers)
	{
		const VkBool32 verbose_value = true;
		const VkLayerSettingEXT layer_setting = { "VK_LAYER_KHRONOS_validation", "printf_verbose", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &verbose_value };
		VkLayerSettingsCreateInfoEXT layer_settings_create_info = { VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT, nullptr, 1, &layer_setting };
		instanceCI.pNext = &layer_settings_create_info;
	}
	

	chk(vkCreateInstance(&instanceCI, nullptr, &instance));
	volkLoadInstance(instance);

	// WINDOW & SURFACE
	window = SDL_CreateWindow("Vulkan Path Tracer", 1980u, 1080u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(window);
	chk(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
	//chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y)); // moving under swapchain logic
	

	// PHYSICAL DEVICE
	std::vector<PhysicalDeviceInfo> physicalDevices = enumerateDevices(instance, surface);
	std::optional<uint32_t> chosen = selectDeviceInteractive(physicalDevices);
	assert(chosen);

	physicalDevice = physicalDevices[*chosen].handle;
	queueFamilies = physicalDevices[*chosen].queueFamily;

	VkPhysicalDeviceProperties2 deviceProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
	vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
	std::cout << "Selected device: " << deviceProperties.properties.deviceName << "\n";

	chk(SDL_Vulkan_GetPresentationSupport(instance, physicalDevice, queueFamilies.main));

	// LOGICAL DEVICE
	std::set<uint32_t> uniqueQueueFamilies{ queueFamilies.main, queueFamilies.asyncCompute, queueFamilies.transfer };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCI{};
		queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCI.queueFamilyIndex = queueFamily;
		queueCI.queueCount = 1;
		queueCI.pQueuePriorities - &queuePriority;
		queueCreateInfos.push_back(queueCI);
	}

	VkPhysicalDeviceVulkan12Features enabledVk12Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	enabledVk12Features.descriptorIndexing = VK_TRUE;
	enabledVk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	enabledVk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
	enabledVk12Features.runtimeDescriptorArray = VK_TRUE;
	enabledVk12Features.bufferDeviceAddress = VK_TRUE;

	VkPhysicalDeviceVulkan13Features enabledVk13Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	enabledVk13Features.pNext = &enabledVk12Features;
	enabledVk13Features.synchronization2 = VK_TRUE;
	enabledVk13Features.dynamicRendering = VK_TRUE;

	VkPhysicalDeviceFeatures enabledVk10Features{};
	enabledVk10Features.samplerAnisotropy = VK_TRUE;
	enabledVk10Features.tessellationShader = VK_TRUE;

	const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo logicalDeviceCI{ .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	logicalDeviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	logicalDeviceCI.pQueueCreateInfos = queueCreateInfos.data();
	logicalDeviceCI.pEnabledFeatures = &enabledVk10Features;
	logicalDeviceCI.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	logicalDeviceCI.ppEnabledExtensionNames = deviceExtensions.data();
	logicalDeviceCI.pNext = &enabledVk13Features;

	chk(vkCreateDevice(physicalDevice, &logicalDeviceCI, nullptr, &logicalDevice));

	vkGetDeviceQueue(logicalDevice, queueFamilies.main, 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, queueFamilies.asyncCompute, 0, &computeQueue);
	vkGetDeviceQueue(logicalDevice, queueFamilies.transfer, 0, &transferQueue);
}

void VulkanDevice::cleanUp()
{
	
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}