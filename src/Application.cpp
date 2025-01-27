#include "Vk/Application.hpp"

#include <algorithm>
#include <cstdlib>

namespace MyVk
{

auto Application::run() noexcept -> void
{
    while (glfwWindowShouldClose(window.win) == 0)
    {
        esc_to_quit();
        glfwPollEvents();
    }
}

auto Application::cWindow() -> void
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (fullscreen)
    {
        GLFWmonitor *monitor{glfwGetPrimaryMonitor()};
        const GLFWvidmode *mode{glfwGetVideoMode(monitor)};
        window.height = mode->height;
        window.width = mode->width;
    }

    window.win = glfwCreateWindow(window.width, window.height, TITLE.c_str(), nullptr, nullptr);

    if (window.win == nullptr)
    {
        throw std::runtime_error("Could not create GLFW window.");
    }
}

auto Application::esc_to_quit() const noexcept -> void
{
    if (glfwGetKey(window.win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window.win, 1);
    }
}

auto Application::init_vulkan() -> void
{
    cInstance();
    if (validation_layers_enabled)
    {
        setup_debug_messenger();
    }
    cPhysicalDevice();
    // New surface.
    if (glfwCreateWindowSurface(instance, window.win, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface.");
    }
    cLogicalDevice();
    cSwapchain();
    cImageViews();
	cRenderPass();
    cGraphicsPipeline();
}

auto Application::clean_up() const noexcept -> void
{
    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);
    std::ranges::for_each(swapchain_image_views.begin(), swapchain_image_views.end(),
                          [this](const auto image_view) { vkDestroyImageView(logical_device, image_view, nullptr); });

    vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
    vkDestroyDevice(logical_device, nullptr);

    if (validation_layers_enabled)
    {
        Application::DestroyDebugUtilsMessengerEXT(instance, debug_messenger);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window.win);
    glfwTerminate();
}

auto Application::cInstance() -> void
{
    if (validation_layers_enabled && !MyVk::Application::check_validation_layer_support())
    {
        throw std::runtime_error("Not all validation layers available.");
    }

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = TITLE.c_str();
    app_info.applicationVersion = VK_API_VER;
    app_info.pEngineName = ENGINE.c_str();
    app_info.engineVersion = VK_API_VER;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo c_instance_info{};
    c_instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    c_instance_info.pApplicationInfo = &app_info;

    uInt32 glfw_ext_count{0};
    c_instance_info.enabledExtensionCount = glfw_ext_count;

    const char **glfwExtensions{glfwGetRequiredInstanceExtensions(&glfw_ext_count)};
    c_instance_info.ppEnabledExtensionNames = glfwExtensions;

    const Vec<cStr> extensions{MyVk::Application::get_required_extensions()};
    c_instance_info.enabledExtensionCount = static_cast<uInt32>(extensions.size());
    c_instance_info.ppEnabledExtensionNames = extensions.data();

    if (validation_layers_enabled)
    {
        c_instance_info.enabledLayerCount = static_cast<uInt32>(validation_layers.size());
        c_instance_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        c_instance_info.enabledLayerCount = 0;
        c_instance_info.pNext = nullptr;
    }

    if (vkCreateInstance(&c_instance_info, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vk instance.");
    }
}
} // namespace MyVk
