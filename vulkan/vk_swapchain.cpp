#include "vk_helpers.h"
#include <GLFW/glfw3.h>
#include <vector>

VulkanSwapchain createSwapchain(GLFWwindow* window, VulkanState vkState){
    VulkanSwapchain swapchain{};

    VK_CHECK(glfwCreateWindowSurface(vkState.instance, window, nullptr, &swapchain.surface));

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkState.physicalDevice, swapchain.surface, &surfaceCaps));
    assert(surfaceCaps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
    assert(surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
    assert(surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    swapchain.extent = surfaceCaps.currentExtent;

    uint32_t surfaceFormatsCount{1};
    VkResult surfaceQueryRes{ vkGetPhysicalDeviceSurfaceFormatsKHR(vkState.physicalDevice, swapchain.surface, &surfaceFormatsCount, &swapchain.surfaceFormat) };
    assert(surfaceQueryRes == VK_SUCCESS || surfaceQueryRes == VK_INCOMPLETE);

    if (swapchain.surfaceFormat.format == VK_FORMAT_UNDEFINED){
        swapchain.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCreateInfo.surface = swapchain.surface;
    swapchainCreateInfo.minImageCount = std::min(2u, surfaceCaps.minImageCount);
    swapchainCreateInfo.imageFormat = swapchain.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapchain.surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent.width = surfaceCaps.currentExtent.width;
    swapchainCreateInfo.imageExtent.height = surfaceCaps.currentExtent.height;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
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

    swapchain.imageViews.resize(swapchainImageCount);
    for (int i{}; i < swapchainImageCount; i++){
        VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        imageViewCreateInfo.image = swapchain.images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapchain.surfaceFormat.format;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        VK_CHECK(vkCreateImageView(vkState.device, &imageViewCreateInfo, nullptr, &swapchain.imageViews[i]));
    }

    return swapchain;
}

void destroySwapchain(VkInstance instance, VkDevice device, VulkanSwapchain& swapchain){
    for (int i{}; i < swapchain.imageViews.size(); i++){
        vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain.swapchain, nullptr);
    vkDestroySurfaceKHR(instance, swapchain.surface, nullptr);
}