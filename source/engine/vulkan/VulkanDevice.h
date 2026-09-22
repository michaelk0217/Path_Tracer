#pragma once

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#ifndef VOLK_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>
#endif

#include "DeviceSelector.h"

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