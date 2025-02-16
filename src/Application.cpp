#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"

namespace MyVk
{

auto Application::triangle() noexcept -> void
{
    while (glfwWindowShouldClose(window.win) == 0)
    {
        esc_to_quit();
        glfwPollEvents();
        draw_frame();
    }
    vkDeviceWaitIdle(logical_device);
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
    glfwSetWindowUserPointer(window.win, this);
    glfwSetFramebufferSizeCallback(window.win, framebufferResizeCallback);

    if (window.win == nullptr)
    {
        throw std::runtime_error("Could not create GLFW window.");
    }
}
// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast, bugprone-easily-swappable-parameters)
auto Application::framebufferResizeCallback(GLFWwindow *window, [[maybe_unused]] int width, [[maybe_unused]] int height)
    -> void
{
    auto *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->frame_buffer_resized = true;
}
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast, bugprone-easily-swappable-parameters)
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
    cDescriptorSetLayout();
    cGraphicsPipeline();
    cFramebuffers();
    cCommandPool();
    cVertexBuffer();
    cIndexBuffer();
    cCommandBuffer();
    cSyncObjects();
}
auto Application::clean_up() noexcept -> void
{
    cleanup_swapchain();

    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);
    vkDestroyRenderPass(logical_device, render_pass, nullptr);

    vkDestroyBuffer(logical_device, index_buffer.first, nullptr);
    vkFreeMemory(logical_device, index_buffer.second, nullptr);
    vkDestroyBuffer(logical_device, vertex_buffer.first, nullptr);
    vkFreeMemory(logical_device, vertex_buffer.second, nullptr);

    vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);

    for (size_t idx = 0; idx < MAX_FRAMES_IN_FLIGHT; idx++)
    {
        vkDestroySemaphore(logical_device, image_available_semaphore[idx], nullptr);
        vkDestroySemaphore(logical_device, render_finished_semaphore[idx], nullptr);
        vkDestroyFence(logical_device, in_flight_fence[idx], nullptr);
    }

    vkDestroyCommandPool(logical_device, command_pool, nullptr);
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
