GLFW_INCLUDE_PATH="/opt/homebrew/include"
VULKAN_INCLUDE_PATH="/Users/bart/VulkanSDK/1.3.224.1/macOS/include"

GLFW_LIBRARY_PATH="/opt/homebrew/lib"
VULKAN_LIBRARY_PATH="/Users/bart/VulkanSDK/1.3.224.1/macOS/lib"

GLFW_LIB="glfw.3.3"
VULKAN_LIB="vulkan.1.3.224"

rm -rf build
mkdir build

clang++ -Wall -g -O0 -std=c++17 -o build/RenderBox \
                                -I$GLFW_INCLUDE_PATH -I$VULKAN_INCLUDE_PATH \
                                -L$GLFW_LIBRARY_PATH -L$VULKAN_LIBRARY_PATH \
                                -l$VULKAN_LIB -l$GLFW_LIB main_macOS.cpp