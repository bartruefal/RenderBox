#include "vk_helpers.h"
#include <GLFW/glfw3.h>
#include <vector>

VulkanState initializeVulkanState(){
    VulkanState vkState{};
    vkState.renderQueueFamilyID = -1;

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.apiVersion = VK_API_VERSION_1_1;

    std::vector<const char*> instanceExtensionNames{};
    uint32_t reqExtCount{};
    const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&reqExtCount);

    for (int i{}; i < reqExtCount; i++){
        instanceExtensionNames.push_back(requiredExtensions[i]);
    }

    instanceExtensionNames.push_back("VK_KHR_portability_enumeration");
    instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    const char* layerNames[]{
        "VK_LAYER_KHRONOS_validation"
    };

    VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = sizeof(layerNames) / sizeof(layerNames[0]);
    instanceInfo.ppEnabledLayerNames = layerNames;
    instanceInfo.enabledExtensionCount = instanceExtensionNames.size();
    instanceInfo.ppEnabledExtensionNames = instanceExtensionNames.data();

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &vkState.instance));

    vkState.debugCallback = registerDebugReport(vkState.instance);

    // TODO: select proper physical device
    uint32_t physDevCount{};
    VK_CHECK(vkEnumeratePhysicalDevices(vkState.instance, &physDevCount, nullptr));
    assert(physDevCount > 0);

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

    const char* deviceExtensionNames[]{
        "VK_KHR_portability_subset",
        "VK_KHR_swapchain",
        "VK_KHR_create_renderpass2"
    };

    VkDeviceCreateInfo devInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueCreateInfo;
    devInfo.enabledExtensionCount = sizeof(deviceExtensionNames) / sizeof(deviceExtensionNames[0]);
    devInfo.ppEnabledExtensionNames = deviceExtensionNames;

    VK_CHECK(vkCreateDevice(vkState.physicalDevice, &devInfo, nullptr, &vkState.device));

    VkDeviceQueueInfo2 queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
    queueInfo.queueIndex = 0;
    queueInfo.queueFamilyIndex = vkState.renderQueueFamilyID;

    vkGetDeviceQueue2(vkState.device, &queueInfo, &vkState.renderQueue);

    return vkState;
}

void destroyVulkanState(VulkanState vkState){
    vkDestroyDevice(vkState.device, nullptr);
    destroyDebugReportCallback(vkState.instance, vkState.debugCallback);
    vkDestroyInstance(vkState.instance, nullptr);
}