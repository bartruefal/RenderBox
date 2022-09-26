#!/bin/bash

if [[ $1 = "-D" ]]
then
    SHADER_COMPILER_ARGS="-V -Od"
    APP_PREPROC_DEFINES="-DRB_DEBUG"
    APP_COMPILER_ARGS="-g -O0"
    BUILD_FOLDER="build/Debug"

    echo "Starting DEBUG build..."
    echo ""
elif [[ $1 = "-R" ]]
then
    SHADER_COMPILER_ARGS="-V"
    APP_PREPROC_DEFINES="-DRB_RELEASE"
    APP_COMPILER_ARGS="-O2"
    BUILD_FOLDER="build/Release"

    echo "Starting RELEASE build..."
    echo ""
else
    echo "Build script should have a debug(\"-D\") or release(\"-R\") argument!"
    exit 0
fi

rm -rf $BUILD_FOLDER
mkdir -p $BUILD_FOLDER/shaders

# build shaders
for filename in shaders/*.vert; do
    name=${filename##*/}
    base=${name%.vert}
    glslangValidator $SHADER_COMPILER_ARGS $filename -o $BUILD_FOLDER/shaders/$base.vs.spv
done

for filename in shaders/*.frag; do
    name=${filename##*/}
    base=${name%.frag}
    glslangValidator $SHADER_COMPILER_ARGS $filename -o $BUILD_FOLDER/shaders/$base.fs.spv
done

# build the app
GLFW_INCLUDE_PATH="/opt/homebrew/include"
VULKAN_INCLUDE_PATH="/Users/bart/VulkanSDK/1.3.224.1/macOS/include"

GLFW_LIBRARY_PATH="/opt/homebrew/lib"
VULKAN_LIBRARY_PATH="/Users/bart/VulkanSDK/1.3.224.1/macOS/lib"

GLFW_LIB="glfw.3.3"
VULKAN_LIB="vulkan.1.3.224"

clang++ -Wall -std=c++17 \
        $APP_COMPILER_ARGS \
        $APP_PREPROC_DEFINE \
        -o $BUILD_FOLDER/RenderBox \
        -I$GLFW_INCLUDE_PATH -I$VULKAN_INCLUDE_PATH \
        -L$GLFW_LIBRARY_PATH -L$VULKAN_LIBRARY_PATH \
        -l$VULKAN_LIB -l$GLFW_LIB main_macOS.cpp