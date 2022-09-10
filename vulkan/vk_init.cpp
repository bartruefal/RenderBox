#include "vk_helpers.h"
#include <GLFW/glfw3.h>
#include <vector>

struct VulkanState{
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSwapchainKHR swapchain;
    VkSurfaceKHR surface;
    VkQueue renderQueue;
    uint32_t renderQueueFamilyID;
};

VulkanState initializeVulkanState(GLFWwindow* window){
    VulkanState vkState{};
    vkState.renderQueueFamilyID = -1;

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_API_VERSION_1_1;

    std::vector<const char*> extNames{};
    uint32_t reqExtCount{};
    const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&reqExtCount);

    for (int i{}; i < reqExtCount; i++){
        extNames.push_back(requiredExtensions[i]);
    }

    extNames.push_back("VK_KHR_portability_enumeration");

    const char* layerNames[]{
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = sizeof(layerNames) / sizeof(layerNames[0]);
    instanceInfo.ppEnabledLayerNames = layerNames;
    instanceInfo.enabledExtensionCount = extNames.size();
    instanceInfo.ppEnabledExtensionNames = extNames.data();

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &vkState.instance));

    // TODO: select proper physical device
    uint32_t physDevCount{};
    VK_CHECK(vkEnumeratePhysicalDevices(vkState.instance, &physDevCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physDevCount);
    VK_CHECK(vkEnumeratePhysicalDevices(vkState.instance, &physDevCount, physicalDevices.data()));

    vkState.physicalDevice = physicalDevices[0];

    uint32_t queueFamilyPropsCount{};
    vkGetPhysicalDeviceQueueFamilyProperties2(vkState.physicalDevice, &queueFamilyPropsCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilyProps(queueFamilyPropsCount);

    for (int i{}; i < queueFamilyPropsCount; i++){
        queueFamilyProps[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(vkState.physicalDevice, &queueFamilyPropsCount, queueFamilyProps.data());

    for (int i{}; i < queueFamilyPropsCount; i++){
        if (queueFamilyProps[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            queueFamilyProps[i].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT){
            vkState.renderQueueFamilyID = i;
            break;
        }
    }

    assert(vkState.renderQueueFamilyID != -1);

    float queuePriorities[]{ 1.0f };

    VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = vkState.renderQueueFamilyID;
    queueCreateInfo.pQueuePriorities = queuePriorities;

    const char* extensionNames[]{
        "VK_KHR_portability_subset",
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo devInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueCreateInfo;
    devInfo.enabledExtensionCount = sizeof(extensionNames) / sizeof(extensionNames[0]);
    devInfo.ppEnabledExtensionNames = extensionNames;

    VK_CHECK(vkCreateDevice(vkState.physicalDevice, &devInfo, nullptr, &vkState.device));

    VkDeviceQueueInfo2 queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
    queueInfo.queueIndex = 0;
    queueInfo.queueFamilyIndex = vkState.renderQueueFamilyID;

    vkGetDeviceQueue2(vkState.device, &queueInfo, &vkState.renderQueue);

    VK_CHECK(glfwCreateWindowSurface(vkState.instance, window, nullptr, &vkState.surface));

    int windowWidth{};
    int windowHeight{};
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCreateInfo.surface = vkState.surface;
    swapchainCreateInfo.minImageCount = 2;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent.width = windowWidth;
    swapchainCreateInfo.imageExtent.height = windowHeight;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainCreateInfo.queueFamilyIndexCount = 1;
    swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t*)&vkState.renderQueueFamilyID;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VK_CHECK(vkCreateSwapchainKHR(vkState.device, &swapchainCreateInfo, nullptr, &vkState.swapchain));

    return vkState;
}