#pragma once

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_MODEL_TEXURES 1
#define EMPTY_IMAGE_FILEPATH "../../../models/backpack/diffuse.jpg"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <array>

#include <boost/serialization/strong_typedef.hpp>