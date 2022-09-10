#ifndef VK_HELPERS_H
#define VK_HELPERS_H

#include <vulkan/vulkan.h>
#include <assert.h>

#define VK_CHECK(vkCall) \
    do { \
        VkResult res{ vkCall }; \
        assert(res == VK_SUCCESS); \
    } while (0)

#endif // VK_HELPERS_H