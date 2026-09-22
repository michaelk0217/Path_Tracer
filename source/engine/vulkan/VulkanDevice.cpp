#include "VulkanDevice.h"




#include <iostream>
#include <cassert>
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

	chk(vkCreateInstance(&instanceCI, nullptr, &instance));
	volkLoadInstance(instance);

	// WINDOW & SURFACE
	window = SDL_CreateWindow("Vulkan Path Tracer", 1980u, 1080u, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	assert(window);
	chk(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface));
	//chk(SDL_GetWindowSize(window, &windowSize.x, &windowSize.y)); // moving under swapchain logic
	

	// DEVICE
	//uint32_t physicalDeviceCount{ 0 };
	//chk(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
	//physicalDevices.resize(physicalDeviceCount);
	//chk(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
	//uint32_t physicalDeviceIndex{ 0 };

	std::vector<PhysicalDeviceInfo> physicalDevices = enumerateDevices(instance, surface);
	std::optional<uint32_t> chosen = selectDeviceInteractive(physicalDevices);
	assert(chosen);

	physicalDevice = physicalDevices[*chosen].handle;
	queueFamilies = physicalDevices[*chosen].queueFamily;
}

void VulkanDevice::cleanUp()
{
	

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	SDL_DestroyWindow(window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}