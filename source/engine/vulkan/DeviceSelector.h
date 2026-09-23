// engine/vulkan/DeviceSelector.h
#pragma once

#include <Volk/volk.h>
#include <vector>
#include <string>
#include <optional>

struct QueueFamilies {
    uint32_t main{ VK_QUEUE_FAMILY_IGNORED };           // graphics + compute + present
    uint32_t transfer{ VK_QUEUE_FAMILY_IGNORED };       // dedicated DMA, if one exists
    uint32_t asyncCompute{ VK_QUEUE_FAMILY_IGNORED };   // compute without graphics, if one exists

    bool hasDedicatedTransfer() const { return transfer != main && transfer != VK_QUEUE_FAMILY_IGNORED; }
    bool hasAsyncCompute()      const { return asyncCompute != VK_QUEUE_FAMILY_IGNORED; }
};

struct PhysicalDeviceInfo {
    VkPhysicalDevice handle{ VK_NULL_HANDLE };
    std::string name;
    VkPhysicalDeviceType type{};
    uint64_t deviceLocalMemory{ 0 };
    //uint32_t graphicsComputeFamily{ VK_QUEUE_FAMILY_IGNORED };
    QueueFamilies queueFamily{};
    bool supportsRayTracing{ false };
    bool suitable{ false };
    std::string rejectReason;
};



std::vector<PhysicalDeviceInfo> enumerateDevices(VkInstance instance, VkSurfaceKHR surface);
std::optional<uint32_t> selectDeviceInteractive(const std::vector<PhysicalDeviceInfo>& devices);
uint32_t selectDeviceAutomatic(const std::vector<PhysicalDeviceInfo>& devices);