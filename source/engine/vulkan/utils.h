#pragma once
#include <Volk/volk.h>
#include <iostream>

static inline void chk(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		std::cerr << "Vulkan call return an error (" << result << ")\n";
		exit(result);
	}
}

static inline void chk(bool result)
{
	if (!result)
	{
		std::cerr << "Call return an error\n";
		exit(result);
	}
}