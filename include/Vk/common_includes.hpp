#pragma once
#ifndef COMMON_INCLUDES_HPP
#define COMMON_INCLUDES_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template <typename T> using Vec = std::vector<T>;
template <typename T> using Set = std::set<T>;
template <typename T, typename U> using MultiMap = std::multimap<T, U>;
template <typename T, int U> using Array = std::array<T, U>;

using Buffer = std::pair<VkBuffer, VkDeviceMemory>;
using string = std::string;
using cStr = const char *;
using uInt32 = uint32_t;
using uInt32_opt = std::optional<uInt32>;

#ifdef NDEBUG
[[maybe_unused]] const bool validation_layers_enabled{false};
#else
[[maybe_unused]] const bool validation_layers_enabled{true};
#endif

namespace MyVk
{
struct Vertex
{
    glm::vec2 pos{};
    glm::vec3 color{};

    static auto get_binding_description() -> VkVertexInputBindingDescription;
    static auto get_attribute_description() -> Array<VkVertexInputAttributeDescription, 2>;
};
} // namespace MyVk
  //
// NOLINTBEGIN(misc-definitions-in-headers)
namespace
{
const string TITLE{"My Triangle"};
const string ENGINE{"No Engine"};
const uInt32 VK_API_VER{VK_MAKE_API_VERSION(0, 1, 0, 0)};
const Vec<cStr> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const Vec<cStr> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const int MAX_FRAMES_IN_FLIGHT{2};
const glm::vec3 ColorIncrement{0.01F, 0.01F, 0.01F};
const Vec<MyVk::Vertex> vertices = {
    // clang-format off
    {{0.0F, -0.5F}, {1.0F, 1.0F, 0.0F}},
    {{0.5F, 0.5F }, {0.0F, 1.0F, 0.0F}},
    {{-0.5F, 0.5F}, {0.0F, 0.0F, 1.0F}}};
// clang-format on

} // namespace
// NOLINTEND(misc-definitions-in-headers)
#endif // COMMON_HPP
