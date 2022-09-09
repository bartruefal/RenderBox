#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>
#include <vector>

#define VK_CHECK(vkCall) \
    do { \
        VkResult res{ vkCall }; \
        assert(res == VK_SUCCESS); \
    } while (0)

static GLFWwindow* gWindow{};
static VkInstance gInstance{VK_NULL_HANDLE};
static VkPhysicalDevice gPhysicalDevice{VK_NULL_HANDLE};
static VkDevice gDevice{VK_NULL_HANDLE};
static VkQueue gQueue{VK_NULL_HANDLE};
static uint32_t gQueueFamilyID{UINT32_MAX};
static VkSwapchainKHR gSwapchain{VK_NULL_HANDLE};
static VkSurfaceKHR gSurface{VK_NULL_HANDLE};

void createInstance(){
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

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &gInstance));

    // TODO: select proper physical device
    uint32_t physDevCount{};
    VK_CHECK(vkEnumeratePhysicalDevices(gInstance, &physDevCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physDevCount);
    VK_CHECK(vkEnumeratePhysicalDevices(gInstance, &physDevCount, physicalDevices.data()));

    gPhysicalDevice = physicalDevices[0];
}

void createDevice(){
    uint32_t queueFamilyPropsCount{};
    vkGetPhysicalDeviceQueueFamilyProperties2(gPhysicalDevice, &queueFamilyPropsCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilyProps(queueFamilyPropsCount);

    for (int i{}; i < queueFamilyPropsCount; i++){
        queueFamilyProps[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(gPhysicalDevice, &queueFamilyPropsCount, queueFamilyProps.data());

    for (int i{}; i < queueFamilyPropsCount; i++){
        if (queueFamilyProps[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            queueFamilyProps[i].queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT){
            gQueueFamilyID = i;
            break;
        }
    }

    assert(gQueueFamilyID != -1);

    float queuePriorities[]{ 1.0f };

    VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.queueFamilyIndex = gQueueFamilyID;
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

    VK_CHECK(vkCreateDevice(gPhysicalDevice, &devInfo, nullptr, &gDevice));

    VkDeviceQueueInfo2 queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2 };
    queueInfo.queueIndex = 0;
    queueInfo.queueFamilyIndex = gQueueFamilyID;

    vkGetDeviceQueue2(gDevice, &queueInfo, &gQueue);

    VK_CHECK(glfwCreateWindowSurface(gInstance, gWindow, nullptr, &gSurface));

    int windowWidth{};
    int windowHeight{};
    glfwGetWindowSize(gWindow, &windowWidth, &windowHeight);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    swapchainCreateInfo.surface = gSurface;
    swapchainCreateInfo.minImageCount = 2;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent.width = windowWidth;
    swapchainCreateInfo.imageExtent.height = windowHeight;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    swapchainCreateInfo.queueFamilyIndexCount = 1;
    swapchainCreateInfo.pQueueFamilyIndices = (const uint32_t*)&gQueueFamilyID;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    vkCreateSwapchainKHR(gDevice, &swapchainCreateInfo, nullptr, &gSwapchain);
}

int main() {
    int glfwInitResult{glfwInit()};
    assert(glfwInitResult == GLFW_TRUE);
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    gWindow = glfwCreateWindow(1024, 768, "Vulkan Test", nullptr, nullptr);
    
    createInstance();
    createDevice();

    uint32_t swapchainImageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(gDevice, gSwapchain, &swapchainImageCount, nullptr));

    std::vector<VkImage> swapchainImages(swapchainImageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(gDevice, gSwapchain, &swapchainImageCount, swapchainImages.data()));

    VkCommandPoolCreateInfo cmdPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    cmdPoolCreateInfo.queueFamilyIndex = gQueueFamilyID;
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkCommandPool cmdPool{VK_NULL_HANDLE};
    VK_CHECK(vkCreateCommandPool(gDevice, &cmdPoolCreateInfo, nullptr, &cmdPool));

    VkCommandBufferAllocateInfo cmdBuffersAllocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cmdBuffersAllocInfo.commandBufferCount = swapchainImageCount;
    cmdBuffersAllocInfo.commandPool = cmdPool;
    cmdBuffersAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    std::vector<VkCommandBuffer> cmdBuffers(swapchainImageCount);
    VK_CHECK(vkAllocateCommandBuffers(gDevice, &cmdBuffersAllocInfo, cmdBuffers.data()));

    VkSemaphoreCreateInfo semCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkSemaphore imageAcquireSemaphore{ VK_NULL_HANDLE };
    VK_CHECK(vkCreateSemaphore(gDevice, &semCreateInfo, nullptr, &imageAcquireSemaphore));

    VkSemaphore imageReleaseSemaphore{ VK_NULL_HANDLE };
    VK_CHECK(vkCreateSemaphore(gDevice, &semCreateInfo, nullptr, &imageReleaseSemaphore));

    VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    std::vector<VkFence> fences(swapchainImageCount);
    for (int i{}; i < swapchainImageCount; i++){
        VK_CHECK(vkCreateFence(gDevice, &fenceCreateInfo, nullptr, &fences[i]));
    }

    while(!glfwWindowShouldClose(gWindow)){
        uint32_t nextImageID{};
        VkResult acquireRes{vkAcquireNextImageKHR(gDevice, gSwapchain, -1, imageAcquireSemaphore, VK_NULL_HANDLE, &nextImageID)};
        assert(acquireRes == VK_SUCCESS || acquireRes == VK_SUBOPTIMAL_KHR);

        VK_CHECK(vkWaitForFences(gDevice, 1, &fences[nextImageID], VK_FALSE, -1));
        VK_CHECK(vkResetFences(gDevice, 1, &fences[nextImageID]));

        VkCommandBufferBeginInfo cmdBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmdBuffers[nextImageID], &cmdBeginInfo));
        {
            VkImageSubresourceRange imageRange{ VK_IMAGE_ASPECT_COLOR_BIT };
            imageRange.levelCount = 1;
            imageRange.layerCount = 1;

            VkImageMemoryBarrier imageMemoryBarrier1{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier1.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            imageMemoryBarrier1.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            imageMemoryBarrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier1.srcQueueFamilyIndex = gQueueFamilyID;
            imageMemoryBarrier1.dstQueueFamilyIndex = gQueueFamilyID;
            imageMemoryBarrier1.image = swapchainImages[nextImageID];
            imageMemoryBarrier1.subresourceRange = imageRange;

            vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_DEPENDENCY_BY_REGION_BIT,
                                 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier1);

            VkClearColorValue clearColor{{0.2, 1, 0.4, 1}};
            vkCmdClearColorImage(cmdBuffers[nextImageID], swapchainImages[nextImageID], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageRange);

            VkImageMemoryBarrier imageMemoryBarrier2{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
            imageMemoryBarrier2.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            imageMemoryBarrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            imageMemoryBarrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            imageMemoryBarrier2.srcQueueFamilyIndex = gQueueFamilyID;
            imageMemoryBarrier2.dstQueueFamilyIndex = gQueueFamilyID;
            imageMemoryBarrier2.image = swapchainImages[nextImageID];
            imageMemoryBarrier2.subresourceRange = imageRange;

            vkCmdPipelineBarrier(cmdBuffers[nextImageID],
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VK_DEPENDENCY_BY_REGION_BIT,
                                 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier2);
        }
        VK_CHECK(vkEndCommandBuffer(cmdBuffers[nextImageID]));

        VkPipelineStageFlags waitStages[]{
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAcquireSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffers[nextImageID];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &imageReleaseSemaphore;

        vkQueueSubmit(gQueue, 1, &submitInfo, fences[nextImageID]);

        VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &imageReleaseSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &gSwapchain;
        presentInfo.pImageIndices = &nextImageID;

        vkQueuePresentKHR(gQueue, &presentInfo);

        glfwPollEvents();
    }
    
    // TODO: Destroy objects

    glfwDestroyWindow(gWindow);
    
    glfwTerminate();
    
    return 0;
}