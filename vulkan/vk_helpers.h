#ifndef VK_HELPERS_H
#define VK_HELPERS_H

#include <vulkan/vulkan.h>
#include <assert.h>
#include <vector>

#define VK_CHECK(vkCall) \
    do { \
        VkResult res{ vkCall }; \
        assert(res == VK_SUCCESS); \
    } while (0)

struct VulkanState{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue renderQueue;
    uint32_t renderQueueFamilyID;
};

struct VulkanSwapchain{
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    std::vector<VkImage> images;
};

#endif // VK_HELPERS_H