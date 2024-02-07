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



find_package(imgui CONFIG REQUIRED)
add_subdirectory(include/dep/MesmerizeRenderer)


add_executable(WaterSimulation "src/main.cpp" "include/private/camera.h")

target_include_directories(WaterSimulation PRIVATE include/private)
target_include_directories(WaterSimulation PRIVATE include/dep)
target_include_directories(WaterSimulation PUBLIC include/public)

target_link_libraries(WaterSimulation PUBLIC imgui::imgui)
target_link_libraries(WaterSimulation PUBLIC MesmerizeRenderer)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET WaterSimulation PROPERTY CXX_STANDARD 20)
endif()

