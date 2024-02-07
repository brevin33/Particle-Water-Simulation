# CMakeList.txt : CMake project for Vulkan-CMake-Template, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.16)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Vulkan-CMake-Template")


find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_library(MyVulkanLib STATIC "src/lib.cpp")

target_include_directories(MyVulkanLib PRIVATE include/private)
target_include_directories(MyVulkanLib PRIVATE include/dep)
target_include_directories(MyVulkanLib PUBLIC include/public)

target_precompile_headers(MyVulkanLib PRIVATE include/private/pch.h)

target_link_libraries(MyVulkanLib PRIVATE glfw)
target_link_libraries(MyVulkanLib PRIVATE Vulkan::Vulkan)
target_link_libraries(MyVulkanLib PRIVATE glm::glm)
target_link_libraries(MyVulkanLib PRIVATE imgui::imgui)
target_link_libraries(MyVulkanLib PRIVATE assimp::assimp)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MyVulkanLib PROPERTY CXX_STANDARD 20)
endif()


# how to use the lib in another application

#add_executable (Vulkan-CMake-Template "example_deleteMe/main.cpp")
#target_link_libraries(Vulkan-CMake-Template PRIVATE MyVulkanLib)