// engine/vulkan/DeviceSelector.cpp
#include "DeviceSelector.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>

static const char* kRequiredExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
    VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
};

static const char* typeName(VkPhysicalDeviceType t) {
    switch (t) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "Discrete";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "Virtual";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
    default:                                     return "Other";
    }
}

static bool hasExtensions(VkPhysicalDevice dev, std::string& missingOut) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, available.data());

    for (const char* required : kRequiredExtensions) {
        const bool found = std::any_of(available.begin(), available.end(),
            [&](const VkExtensionProperties& e) { return std::strcmp(e.extensionName, required) == 0; });
        if (!found) { missingOut = required; return false; }
    }
    return true;
}

//static uint32_t findGraphicsComputeQueue(VkPhysicalDevice dev, VkSurfaceKHR surface) {
//    uint32_t count = 0;
//    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
//    std::vector<VkQueueFamilyProperties> families(count);
//    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families.data());
//
//    for (uint32_t i = 0; i < count; ++i) {
//        const bool gfx = families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
//        const bool cmp = families[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
//        if (!gfx || !cmp) continue;
//
//        VkBool32 present = VK_FALSE;
//        if (surface != VK_NULL_HANDLE)
//            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present);
//        if (surface == VK_NULL_HANDLE || present)
//            return i;
//    }
//    return VK_QUEUE_FAMILY_IGNORED;
//}

static QueueFamilies findQueueFamilies(VkPhysicalDevice dev, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families.data());

    QueueFamilies q{};

    for (uint32_t i = 0; i < count; ++i) {
        const VkQueueFlags flags = families[i].queueFlags;
        const bool gfx = flags & VK_QUEUE_GRAPHICS_BIT;
        const bool compute = flags & VK_QUEUE_COMPUTE_BIT;
        const bool transfer = flags & VK_QUEUE_TRANSFER_BIT;

        // Main: graphics + compute + present
        if (q.main == VK_QUEUE_FAMILY_IGNORED && gfx && compute) {
            VkBool32 present = VK_FALSE;
            if (surface != VK_NULL_HANDLE)
                vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present);
            if (surface == VK_NULL_HANDLE || present) q.main = i;
        }

        // Dedicated transfer: transfer-capable, no graphics, no compute
        if (transfer && !gfx && !compute && q.transfer == VK_QUEUE_FAMILY_IGNORED)
            q.transfer = i;

        // Async compute: compute without graphics
        if (compute && !gfx && q.asyncCompute == VK_QUEUE_FAMILY_IGNORED)
            q.asyncCompute = i;
    }

    // Fall back to the main family if no dedicated transfer engine exists
    if (q.transfer == VK_QUEUE_FAMILY_IGNORED) q.transfer = q.main;

    return q;
}

std::vector<PhysicalDeviceInfo> enumerateDevices(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> handles(count);
    vkEnumeratePhysicalDevices(instance, &count, handles.data());

    std::vector<PhysicalDeviceInfo> out;
    out.reserve(count);

    for (VkPhysicalDevice handle : handles) {
        PhysicalDeviceInfo info{};
        info.handle = handle;

        VkPhysicalDeviceProperties2 props{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        vkGetPhysicalDeviceProperties2(handle, &props);
        info.name = props.properties.deviceName;
        info.type = props.properties.deviceType;

        VkPhysicalDeviceMemoryProperties mem{};
        vkGetPhysicalDeviceMemoryProperties(handle, &mem);
        for (uint32_t i = 0; i < mem.memoryHeapCount; ++i)
            if (mem.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                info.deviceLocalMemory = std::max(info.deviceLocalMemory, mem.memoryHeaps[i].size);

        // Feature chain
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rt{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accel{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &rt };
        VkPhysicalDeviceVulkan12Features v12{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &accel };
        VkPhysicalDeviceFeatures2 features{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &v12 };
        vkGetPhysicalDeviceFeatures2(handle, &features);

        info.supportsRayTracing = rt.rayTracingPipeline && accel.accelerationStructure
            && v12.bufferDeviceAddress;
        //info.graphicsComputeFamily = findGraphicsComputeQueue(handle, surface);
        info.queueFamily = findQueueFamilies(handle, surface);

        std::string missing;
        if (props.properties.apiVersion < VK_API_VERSION_1_2)
            info.rejectReason = "Vulkan 1.2 required";
        else if (!hasExtensions(handle, missing))
            info.rejectReason = "missing " + missing;
        else if (!info.supportsRayTracing)
            info.rejectReason = "no ray tracing pipeline support";
        else if (info.queueFamily.main == VK_QUEUE_FAMILY_IGNORED)
            info.rejectReason = "no graphics+compute+present queue";
        else
            info.suitable = true;

        out.push_back(std::move(info));
    }
    return out;
}

uint32_t selectDeviceAutomatic(const std::vector<PhysicalDeviceInfo>& devices) {
    uint32_t best = UINT32_MAX;
    uint64_t bestScore = 0;
    for (uint32_t i = 0; i < devices.size(); ++i) {
        if (!devices[i].suitable) continue;
        uint64_t score = devices[i].deviceLocalMemory;
        if (devices[i].type == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += (1ull << 40);
        if (score > bestScore) { bestScore = score; best = i; }
    }
    return best;
}

std::optional<uint32_t> selectDeviceInteractive(const std::vector<PhysicalDeviceInfo>& devices) {
    std::printf("\nAvailable devices:\n");
    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& d = devices[i];
        std::printf("  [%zu] %-40s %-10s %5.1f GB  %s\n",
            i, d.name.c_str(), typeName(d.type),
            d.deviceLocalMemory / (1024.0 * 1024.0 * 1024.0),
            d.suitable ? "OK" : ("UNSUPPORTED: " + d.rejectReason).c_str());
    }

    const uint32_t fallback = selectDeviceAutomatic(devices);
    if (fallback == UINT32_MAX) {
        std::printf("No device supports ray tracing.\n");
        return std::nullopt;
    }

    std::printf("Select device [Enter for %u]: ", fallback);
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) return fallback;

    try {
        const uint32_t choice = static_cast<uint32_t>(std::stoul(line));
        if (choice < devices.size() && devices[choice].suitable) return choice;
    }
    catch (...) {}

    std::printf("Invalid choice, using %u.\n", fallback);
    return fallback;
}