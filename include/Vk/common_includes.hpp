#pragma once
#ifndef COMMON_INCLUDES_HPP
#define COMMON_INCLUDES_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
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
using string = std::string;
using cStr = const char *;
using uInt32 = uint32_t;
using uInt32_opt = std::optional<uInt32>;

#ifdef NDEBUG
[[maybe_unused]] const bool validation_layers_enabled{false};
#else
[[maybe_unused]] const bool validation_layers_enabled{true};
#endif

// NOLINTBEGIN(misc-definitions-in-headers)
namespace
{
const string TITLE{"Triangle"};
const string ENGINE{"No Engine"};
const uInt32 VK_API_VER{VK_MAKE_API_VERSION(0, 1, 0, 0)};
const Vec<cStr> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const Vec<cStr> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

} // namespace
// NOLINTEND(misc-definitions-in-headers)
#endif // COMMON_HPP
