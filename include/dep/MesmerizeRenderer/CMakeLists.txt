# CMakeList.txt : CMake project for Vulkan-CMake-Template, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.16)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Mesmerize-Renderer")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )


add_library(MesmerizeRenderer STATIC  "src/VulkanRenderer.cpp" "src/GenericRenderer/Defaults.cpp" "src/GenericRenderer/Renderer.cpp")

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
target_include_directories(MesmerizeRenderer PRIVATE include/private)
target_include_directories(MesmerizeRenderer PRIVATE include/dep)
target_include_directories(MesmerizeRenderer PUBLIC include/public)

target_precompile_headers(MesmerizeRenderer PUBLIC include/private/pch.h)

add_compile_definitions(VULKANRENDERER)

target_link_libraries(MesmerizeRenderer PUBLIC glfw)
target_link_libraries(MesmerizeRenderer PUBLIC glm::glm)
target_link_libraries(MesmerizeRenderer PRIVATE Vulkan::Vulkan)
target_link_libraries(MesmerizeRenderer PUBLIC Vulkan::Headers GPUOpen::VulkanMemoryAllocator)
TARGET_LINK_LIBRARIES(MesmerizeRenderer LINK_PUBLIC ${Boost_LIBRARIES} )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MesmerizeRenderer PROPERTY CXX_STANDARD 20)
endif()


# build example folder
find_package(assimp CONFIG)
add_executable (exampleModel "example/modelLoading/main.cpp" "example/modelLoading/Model.cpp")
target_link_libraries(exampleModel PRIVATE MesmerizeRenderer)
target_link_libraries(exampleModel PUBLIC assimp::assimp)


add_executable (exampleTessalization "example/tessalization/main.cpp")
target_link_libraries(exampleTessalization PRIVATE MesmerizeRenderer)
