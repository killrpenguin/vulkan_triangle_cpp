#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"

namespace MyVk
{

auto Application::check_device_exts_support() const noexcept -> bool
{
    uInt32 extension_cnt{};
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_cnt, nullptr);

    Vec<VkExtensionProperties> available_extensions(extension_cnt);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_cnt, available_extensions.data());

    Set<string> required_exts(device_extensions.begin(), device_extensions.end());

    for (const auto &ext : available_extensions)
    {
        // reinterpret_cast ?
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        required_exts.erase(ext.extensionName);
        // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    }

    return required_exts.empty();
}

auto Application::cPhysicalDevice() -> void
{
    uInt32 device_cnt{0};
    vkEnumeratePhysicalDevices(instance, &device_cnt, nullptr);

    if (device_cnt == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support.");
    }

    Vec<VkPhysicalDevice> devices(device_cnt);
    vkEnumeratePhysicalDevices(instance, &device_cnt, devices.data());

    physical_device = find_device(devices);

    if (physical_device == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU.");
    }
}

auto Application::find_device(const Vec<VkPhysicalDevice> &devices) noexcept -> VkPhysicalDevice
{
    switch (devices.size())
    {
    case 0:
        return VK_NULL_HANDLE;
    case 1:
        return devices[0];
    default:
        break;
    }

    auto dev{devices.begin()};
    MultiMap<int, VkPhysicalDevice> candidates{};

    while (dev != devices.end())
    {
        uInt32 score{0};

        VkPhysicalDeviceProperties device_properties{};
        vkGetPhysicalDeviceProperties(*dev, &device_properties);

        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(*dev, &deviceFeatures);

        score += device_properties.limits.maxImageDimension2D;

        if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            score++;
        }
        if (deviceFeatures.geometryShader == 0U)
        {
            score++;
        }
        candidates.insert(std::make_pair(score, *dev));
        dev++;
    }
    return candidates.rbegin()->second;
}

template <VkQueueFlagBits queue_flag> auto Application::find_queue_families() const noexcept -> QueueIndices
{
    uInt32 queue_family_cnt{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_cnt, nullptr);

    Vec<VkQueueFamilyProperties> queue_families(queue_family_cnt);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_cnt, queue_families.data());

    QueueIndices queue_indices{};
    for (unsigned long num{0}; num <= queue_families.size(); num++)
    {
        VkBool32 present_support{false};
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, num, surface, &present_support);

        // NOLINTBEGIN(readability-implicit-bool-conversion)
        if (queue_families[num].queueFlags & queue_flag && present_support)
        // NOLINTEND(readability-implicit-bool-conversion)
        {
            const SwapChainSupportDetails swapchain_support{query_swapchain_support()};
            if (check_device_exts_support() && swapchain_support.swapchain_adequate())
            {
                queue_indices.graphics_indices = static_cast<uInt32>(num);
                queue_indices.present_indice = static_cast<uInt32>(num);
                return queue_indices;
            }
        }
    }

    return queue_indices;
}

auto Application::cLogicalDevice() -> void
{
    const QueueIndices indices{find_queue_families<VK_QUEUE_GRAPHICS_BIT>()};
    const uInt32 graphics_indices{indices.graphics_indices.has_value()
                                      ? indices.graphics_indices.value()
                                      : throw std::runtime_error("Failed to find graphics indices in queue family.")};
    const uInt32 present_indices{indices.present_indice.has_value()
                                     ? indices.present_indice.value()
                                     : throw std::runtime_error("Failed to find present indices in queue family.")};

    const Set<uInt32> unique_queue_families = {graphics_indices, present_indices};

    const float queue_priority = 1.0F;
    Vec<VkDeviceQueueCreateInfo> queue_create_info_list{};
    for (uInt32 queue_indices : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_indices; // must be less than queuefamily propertycount
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_info_list.push_back(queue_create_info);
    }
    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uInt32>(queue_create_info_list.size());
    create_info.pQueueCreateInfos = queue_create_info_list.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uInt32>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (validation_layers_enabled)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
    {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &logical_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create logical device.");
    }

    vkGetDeviceQueue(logical_device, graphics_indices, 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, present_indices, 0, &present_queue);
}

template auto Application::find_queue_families<VK_QUEUE_GRAPHICS_BIT>() const noexcept -> QueueIndices;

} // namespace MyVk
