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

VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
                             uint64_t object, size_t location, int32_t messageCode,
                             const char* pLayerPrefix, const char* pMessage, void* pUserData){
    const char* errorType = (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0 ? "ERROR" :
                            (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0 ? "WARNING" :
                            (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0 ? "PERFORMANCE WARNING" :
                            "UNKNOWN";

    printf("%s: %s\n", errorType, pMessage);

    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0){
        assert(!"Validation error encountered!");
    }
    
    return VK_FALSE;
}

VkDebugReportCallbackEXT registerDebugReport(VkInstance instance){
    VkDebugReportCallbackCreateInfoEXT callbackInfo{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
    callbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    callbackInfo.pfnCallback = debugReportCallback;

    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT{ (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT") };
    assert(vkCreateDebugReportCallbackEXT);

    VkDebugReportCallbackEXT debugCallback{};
    VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &callbackInfo, nullptr, &debugCallback));

    return debugCallback;
}

void destroyDebugReportCallback(VkInstance instance, VkDebugReportCallbackEXT debugCallback){
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT{ (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT") };
    assert(vkDestroyDebugReportCallbackEXT);

    vkDestroyDebugReportCallbackEXT(instance, debugCallback, nullptr);
}

struct VulkanState{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkDebugReportCallbackEXT debugCallback;
    VkQueue renderQueue;
    uint32_t renderQueueFamilyID;
};

struct VulkanSwapchain{
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    VkExtent2D extent;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
};

struct GraphicsPipeline{
    VkPipeline pipeline;
    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
};

#endif // VK_HELPERS_H