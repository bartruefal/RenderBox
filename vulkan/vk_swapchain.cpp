#include "vk_helpers.h"
#include <GLFW/glfw3.h>
#include <vector>

VulkanSwapchain createSwapchain(GLFWwindow* window, VulkanState vkState){
    VulkanSwapchain swapchain{};

    VK_CHECK(glfwCreateWindowSurface(vkState.instance, window, nullptr, &swapchain.surface));

    int windowWidth{};
    int windowHeight{};
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    uint32_t surfaceFormatsCount{1};
    VkResult surfaceQueryRes{ vkGetPhysicalDeviceSurfaceFormatsKHR(vkState.physicalDevice, swapchain.surface, &surfaceFormatsCount, &swapchain.surfaceFormat) };
    assert(surfaceQueryRes == VK_SUCCESS || surfaceQueryRes == VK_INCOMPLETE);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCreateInfo.surface = swapchain.surface;
    swapchainCreateInfo.minImageCount = 2;
    swapchainCreateInfo.imageFormat = swapchain.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchain.surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent.width = windowWidth;
    swapchainCreateInfo.imageExtent.height = windowHeight;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainCreateInfo.queueFamilyIndexCount = 1;
    swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t*)&vkState.renderQueueFamilyID;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CHECK(vkCreateSwapchainKHR(vkState.device, &swapchainCreateInfo, nullptr, &swapchain.swapchain));

    uint32_t swapchainImageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(vkState.device, swapchain.swapchain, &swapchainImageCount, nullptr));
    assert(swapchainImageCount > 0);

    swapchain.images.resize(swapchainImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(vkState.device, swapchain.swapchain, &swapchainImageCount, swapchain.images.data()));

    return swapchain;
}