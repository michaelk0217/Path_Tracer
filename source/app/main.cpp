#include <Volk/volk.h>
#include <SDL3/SDL.h>
#include <ktx.h>
#include <slang/slang.h>
#include <glm/glm.hpp>
#include <cstdio>
#include <memory>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "../engine/vulkan/VulkanDevice.h"


int main() {
    // volk
    if (volkInitialize() != VK_SUCCESS) { std::printf("volk: no Vulkan loader\n"); return 1; }
    uint32_t apiVersion = volkGetInstanceVersion();
    std::printf("Vulkan %u.%u\n", VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion));

    // SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::printf("SDL: %s\n", SDL_GetError()); return 1; }
    std::printf("SDL %d\n", SDL_GetVersion());
    SDL_Quit();

    // KTX
    // std::printf("KTX %s\n", ktxVersionString());
    std::printf("KTX: %s\n", ktxErrorString(KTX_SUCCESS));

    // Slang
    slang::IGlobalSession* session = nullptr;
    slang::createGlobalSession(&session);
    std::printf("Slang session %s\n", session ? "ok" : "failed");
    if (session) session->release();

    // glm
    glm::vec3 v = glm::normalize(glm::vec3(1, 2, 3));
    std::printf("glm %.3f\n", v.x);

    std::printf("Shaders at: %s\n", SHADER_DIR);

    std::unique_ptr<VulkanDevice> vulkanDevice = std::make_unique<VulkanDevice>();
    
    vulkanDevice.reset();
    return 0;
}