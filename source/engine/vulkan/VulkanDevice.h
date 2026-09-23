#pragma once

#include <vector>
#include <optional>

#include <Volk/volk.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "DeviceSelector.h"

#ifdef NDEBUG
inline constexpr bool enableValidationLayers = false;
#else
inline constexpr bool enableValidationLayers = true;
#endif 


class VulkanDevice
{
public:

	VkInstance instance{ VK_NULL_HANDLE };

	VkSurfaceKHR surface{ VK_NULL_HANDLE };
	SDL_Window* window{ nullptr };


	//std::vector<VkPhysicalDevice> physicalDevices{};
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };

	VkDevice logicalDevice{ VK_NULL_HANDLE };

	QueueFamilies queueFamilies{};

	VkCommandPool graphicsCommandPool{ VK_NULL_HANDLE };
	VkCommandPool computeCommandPool{ VK_NULL_HANDLE };
	VkCommandPool transferCommandPool{ VK_NULL_HANDLE };

	VulkanDevice();
	~VulkanDevice();

private:

	void initialize();
	void cleanUp();
};