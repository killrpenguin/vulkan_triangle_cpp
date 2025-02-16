#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"
#include <vulkan/vulkan_core.h>

namespace MyVk
{
auto Application::cSwapchain() -> void
{
    QueueIndices indices = find_queue_families<VK_QUEUE_GRAPHICS_BIT>();
    std::array<uInt32, 2> queue_family_indices{
        indices.graphics_indices.has_value()
            ? indices.graphics_indices.value()
            : throw std::runtime_error("Failed to find graphics indices in queue family."),
        indices.present_indice.has_value()
            ? indices.present_indice.value()
            : throw std::runtime_error("Failed to find present indices in queue family.")};

    const SwapChainSupportDetails swapchain_support = query_swapchain_support();

    const VkSurfaceFormatKHR surface_format{choose_swap_surface_format(swapchain_support.formats)};
    const VkPresentModeKHR present_mode{choose_swap_present_mode(swapchain_support.present_modes)};
    const VkExtent2D extent{choose_swap_extent(swapchain_support.capabilities)};

    uInt32 image_cnt{swapchain_support.capabilities.minImageCount + 1};
    if (swapchain_support.capabilities.maxImageCount > 0 && image_cnt > swapchain_support.capabilities.maxImageCount)
    {
        image_cnt = swapchain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = nullptr;
    create_info.flags = 0;
    create_info.surface = surface;
    create_info.minImageCount = image_cnt;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (indices.graphics_indices != indices.present_indice)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices.data();
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_cnt, nullptr);
    swapchain_images.resize(image_cnt);
    vkGetSwapchainImagesKHR(logical_device, swapchain, &image_cnt, swapchain_images.data());
    swapchain_extent = extent;
    swapchain_image_format = surface_format.format;
}
auto Application::cleanup_swapchain() -> void
{
    std::ranges::for_each(swapchain_framebuffer.begin(), swapchain_framebuffer.end(),
                          [this](const auto image_view) { vkDestroyFramebuffer(logical_device, image_view, nullptr); });

    std::ranges::for_each(swapchain_image_views.begin(), swapchain_image_views.end(),
                          [this](const auto image_view) { vkDestroyImageView(logical_device, image_view, nullptr); });
    vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
}
auto Application::recreateSwapchain() -> void
{
    int width{};
    int height{};
    glfwGetFramebufferSize(window.win, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window.win, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(logical_device);

    cSwapchain();
    cImageViews();
    cFramebuffers();
}
auto Application::query_swapchain_support() const noexcept -> SwapChainSupportDetails
{
    SwapChainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

    uInt32 format_cnt{};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_cnt, nullptr);

    if (format_cnt != 0)
    {
        details.formats.resize(format_cnt);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_cnt, details.formats.data());
    }

    uInt32 present_mode_cnt{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_cnt, nullptr);

    if (present_mode_cnt != 0)
    {
        details.present_modes.resize(present_mode_cnt);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_cnt,
                                                  details.present_modes.data());
    }

    return details;
}
auto Application::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept -> VkExtent2D
{
    if (capabilities.currentExtent.width != std::numeric_limits<uInt32>::max())
    {
        return capabilities.currentExtent;
    }

    int width{};
    int height{};
    glfwGetFramebufferSize(window.win, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

auto Application::choose_swap_surface_format(const Vec<VkSurfaceFormatKHR> &available_formats) noexcept
    -> VkSurfaceFormatKHR
{
    // clang-format off
  auto surface_format{std::ranges::find_if(available_formats.begin(), available_formats.end(),
									 [](const auto &format) {return format.format == VK_FORMAT_B8G8R8A8_SRGB
											   && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;})};
    // clang-format on
    if (surface_format != available_formats.end())
    {
        return *surface_format;
    }
    return available_formats[0];
}

auto Application::choose_swap_present_mode(const Vec<VkPresentModeKHR> &available_presepent_modes) noexcept
    -> VkPresentModeKHR
{
    auto mode{std::ranges::find_if(available_presepent_modes.begin(), available_presepent_modes.end(),
                                   [](const auto &mode) { return mode == VK_PRESENT_MODE_MAILBOX_KHR; })};

    if (mode != available_presepent_modes.end())
    {
        return *mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}
auto Application::cImageViews() -> void
{
    swapchain_image_views.resize(swapchain_images.size());

    for (unsigned long idx{0}; idx < swapchain_images.size(); idx++)
    {
        const VkImageViewCreateInfo create_info{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
												.pNext = nullptr,
												.flags = 0,
                                                .image = swapchain_images[idx],
                                                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                                .format = swapchain_image_format,
                                                .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                               .a = VK_COMPONENT_SWIZZLE_IDENTITY},
                                                .subresourceRange =
                                                    {
                                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                        .baseMipLevel = 0,
                                                        .levelCount = 1,
                                                        .baseArrayLayer = 0,
                                                        .layerCount = 1,
                                                    }

        };

        if (vkCreateImageView(logical_device, &create_info, nullptr, &swapchain_image_views[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image views.");
        }
    }
}
} // namespace MyVk
