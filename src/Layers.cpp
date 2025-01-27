#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace MyVk
{
// clang-format off
auto Application::CreateDebugUtilsMessengerEXT(VkInstance instance,
											const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                            VkDebugUtilsMessengerEXT *pDebugMessenger) -> VkResult
// clang-format on
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto func{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, nullptr, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}
// clang-format off
auto Application::DestroyDebugUtilsMessengerEXT(VkInstance instance,
											 VkDebugUtilsMessengerEXT debugMessenger) -> void
// clang-format on
{
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    auto func{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))};
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    if (func != nullptr)
    {
        func(instance, debugMessenger, nullptr);
    }
}

auto Application::get_required_extensions() -> Vec<cStr>
{
    uInt32 glfw_ext_count{0};
    const char **glfw_extensions{glfwGetRequiredInstanceExtensions(&glfw_ext_count)};

    // NOLINTNEXTLINE
    Vec<cStr> extensions(glfw_extensions, glfw_extensions + glfw_ext_count);

    if (validation_layers_enabled)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

auto Application::check_validation_layer_support() -> bool
{
    uInt32 layer_cnt{0};
    vkEnumerateInstanceLayerProperties(&layer_cnt, nullptr);

    Vec<VkLayerProperties> available_layers(layer_cnt);
    vkEnumerateInstanceLayerProperties(&layer_cnt, available_layers.data());

    for (const char *layerName : validation_layers)
    {
        bool layer_found = false;
        for (const auto &layer_properties : available_layers)
        {
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            if (strcmp(layerName, layer_properties.layerName) == 0)
            // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
            {
                layer_found = true;
                break;
            }
        }
        if (!layer_found)
        {
            return false;
        }
    }
    return true;
}

auto Application::setup_debug_messenger() -> void
{
    if (!validation_layers_enabled)
    {
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT create_info{};

    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = nullptr;

    if (Application::CreateDebugUtilsMessengerEXT(instance, &create_info, &debug_messenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

auto Application::debug_callback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                 [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                                 [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                 [[maybe_unused]] void *pUserData) -> VKAPI_ATTR VkBool32 VKAPI_CALL
{
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";
    return VK_FALSE;
}
} // namespace MyVk
