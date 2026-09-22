#pragma once

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;

	const bool isComplete()
	{
		return graphicsFamily.has_value() && computeFamily.has_value() && transferFamily.has_value();
	}
};

class VulkanDevice
{
public:

	VkInstance instance{ VK_NULL_HANDLE };

	VkSurfaceKHR surface{ VK_NULL_HANDLE };

	std::vector<VkPhysicalDevice> physicalDevices{};

	VkDevice logicalDevice{ VK_NULL_HANDLE };

	VkQueue graphicsQueue{ VK_NULL_HANDLE };
	VkQueue computeQueue{ VK_NULL_HANDLE };
	VkQueue transferQueue{ VK_NULL_HANDLE };
	QueueFamilyIndices queueFamilyIndices;

	VkCommandPool graphicsCommandPool{ VK_NULL_HANDLE };
	VkCommandPool computeCommandPool{ VK_NULL_HANDLE };
	VkCommandPool transferCommandPool{ VK_NULL_HANDLE };

	VulkanDevice();
	~VulkanDevice();

private:

	void initialize();
	void clean_up();
};