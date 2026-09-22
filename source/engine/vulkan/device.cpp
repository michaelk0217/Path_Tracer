#include "device.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>

#include "utils.h"

VulkanDevice::VulkanDevice()
{
	
}

VulkanDevice::~VulkanDevice()
{
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


}

void VulkanDevice::clean_up()
{
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}