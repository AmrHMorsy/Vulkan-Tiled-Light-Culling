cmake_minimum_required(VERSION 3.8)

set(EXECUTABLE_NAME "LightCullingDemo")
set(TARGET_NAME LightCullingDemo)

project(TARGET_NAME)

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif()

set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH /encs/opt/local)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
set(VULKAN_SDK "${CMAKE_SOURCE_DIR}/Resources/external/lib/VulkanSDK/macOS")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/include")
set(Vulkan_LIBRARY "${VULKAN_SDK}/lib/libvulkan.dylib")
set(MOLTENVK_LIB "${VULKAN_SDK}/lib/libMoltenVK.dylib")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/Resources/external/lib/imgui")
set(IMGUI_FILE_DIR "${CMAKE_SOURCE_DIR}/Resources/external/lib/ImGuiFileDialog")
set(SPIRV_CROSS_PATH "${VULKAN_SDK}")



set(EXTERNAL_LIBS_DIR ${CMAKE_SOURCE_DIR}/Resources/external)
set(ASSIMP_INCLUDE_DIR ${EXTERNAL_LIBS_DIR}/assimp/build_universal/include)
set(ASSIMP_LIB ${EXTERNAL_LIBS_DIR}/assimp/build_universal/lib/libassimp.a)

set(GLFW_INCLUDE_DIR ${EXTERNAL_LIBS_DIR}/glfw/include/GLFW/)
set(GLFW_LIB ${EXTERNAL_LIBS_DIR}/glfw/build_universal/src/libglfw3.a)









find_package(GLUT REQUIRED)


include_directories(${CMAKE_SOURCE_DIR}/Resources/external/Include)
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(/opt/homebrew/include)
include_directories(src/)
include_directories(${IMGUI_DIR})
include_directories(${SPIRV_CROSS_PATH}/include)
include_directories(${ASSIMP_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})


link_directories(${SPIRV_CROSS_PATH}/lib)
link_directories(${Vulkan_LIBRARY_DIR})


set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_FILE_DIR}/ImGuiFileDialog.cpp
)
set(IMGUI_BACKEND_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

aux_source_directory(src SOURCE)
aux_source_directory(${CMAKE_SOURCE_DIR}/Resources/external/include/MeshOptimizer MESH_OPTIMIZER_SRC)
file(GLOB_RECURSE SRC "src/*.cpp")

set(SOURCE_FILES ${SOURCE} ${SRC} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES} ${MESH_OPTIMIZER_SRC})

add_executable("${TARGET_NAME}" src/Main.cpp ${SOURCE_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME "${EXECUTABLE_NAME}")

target_link_libraries("${TARGET_NAME}"
    "${Vulkan_LIBRARY}"
    "${MOLTENVK_LIB}"
    ${GLUT_LIBRARIES}
    spirv-cross-core
    spirv-cross-glsl
    ${ASSIMP_LIB}
    ${GLFW_LIB}
        # System frameworks for GLFW
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation"
    "-framework CoreGraphics"
    "-framework QuartzCore"
    "-framework CoreServices"
    "-framework AppKit"
    "-framework CoreVideo"
    "-framework AudioToolbox"

    # System zlib for Assimp
    z
)

target_include_directories("${TARGET_NAME}" PRIVATE
                           ${ASSIMP_INCLUDE_DIR}
                           ${EIGEN3_INCLUDE_DIR}
                           ${GLUT_INCLUDE_DIRS}
                           ${Vulkan_INCLUDE_DIR}
                           ${GLFW_INCLUDE_DIR})
