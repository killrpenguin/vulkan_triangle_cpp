#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"

namespace MyVk
{
auto Application::cFramebuffers() -> void
{
    swapchain_framebuffer.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++)
    {
        Array<VkImageView, 1> attachments = {swapchain_image_views[i]};

        const VkFramebufferCreateInfo framebuffer_info{.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                                       .pNext = nullptr,
                                                       .flags = 0,
                                                       .renderPass = render_pass,
                                                       .attachmentCount = 1,
                                                       .pAttachments = attachments.data(),
                                                       .width = swapchain_extent.width,
                                                       .height = swapchain_extent.height,
                                                       .layers = 1};

        if (vkCreateFramebuffer(logical_device, &framebuffer_info, nullptr, &swapchain_framebuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create framebuffer.");
        }
    }
}
auto Application::cCommandPool() -> void
{

    const QueueIndices indices{find_queue_families<VK_QUEUE_GRAPHICS_BIT>()};

    Array<uInt32, 2> queue_family_indices{
        indices.graphics_indices.has_value()
            ? indices.graphics_indices.value()
            : throw std::runtime_error("Failed to find graphics indices in queue family."),
        indices.present_indice.has_value()
            ? indices.present_indice.value()
            : throw std::runtime_error("Failed to find present indices in queue family.")};

    const VkCommandPoolCreateInfo pool_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_family_indices[0],
    };

    if (vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}
auto Application::cCommandBuffer() -> void
{
    command_buffer.resize(MAX_FRAMES_IN_FLIGHT);
    const VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uInt32>(command_buffer.size()),
    };

    if (vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffer.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
auto Application::record_command_buffer(const VkCommandBuffer buffer, const uInt32 image_index) -> void
{
    const VkCommandBufferBeginInfo begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    const VkClearValue clear_color = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    const VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass,
        .framebuffer = swapchain_framebuffer[image_index],
        .renderArea = {.offset = {.x = 0, .y = 0}, .extent = swapchain_extent},
        .clearValueCount = 1,
        .pClearValues = &clear_color,
    };

    vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    const VkViewport viewport{.x = 0.0F,
                              .y = 0.0F,
                              .width = static_cast<float>(swapchain_extent.width),
                              .height = static_cast<float>(swapchain_extent.height),
                              .minDepth = 0.0F,
                              .maxDepth = 1.0F};

    vkCmdSetViewport(buffer, 0, 1, &viewport);

    const VkRect2D scissor{
        .offset = {.x = 0, .y = 0},
        .extent = swapchain_extent,
    };

    vkCmdSetScissor(buffer, 0, 1, &scissor);

    Array<VkBuffer, 1> vertex_buffers_array{vertex_buffer.first};
    Array<VkDeviceSize, 1> offsets_array{0};

    vkCmdBindVertexBuffers(buffer, 0, 1, vertex_buffers_array.data(), offsets_array.data());
    vkCmdBindIndexBuffer(buffer, index_buffer.first, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(buffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(buffer);

    if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

auto Application::cSyncObjects() -> void
{
    image_available_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fence.resize(MAX_FRAMES_IN_FLIGHT);

    const VkSemaphoreCreateInfo semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = nullptr, .flags = 0};

    VkFenceCreateInfo fence_info{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t idx = 0; idx < MAX_FRAMES_IN_FLIGHT; idx++)
    {
        if (vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &image_available_semaphore[idx]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(logical_device, &semaphore_info, nullptr, &render_finished_semaphore[idx]) !=
                VK_SUCCESS ||
            vkCreateFence(logical_device, &fence_info, nullptr, &in_flight_fence[idx]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create semaphores.");
        }
    }
}
auto Application::draw_frame() -> void
{
    vkWaitForFences(logical_device, 1, &in_flight_fence[current_frame], VK_TRUE, UINT64_MAX);

    uInt32 image_idx{};
    const VkResult next_image_result{vkAcquireNextImageKHR(
        logical_device, swapchain, UINT64_MAX, image_available_semaphore[current_frame], VK_NULL_HANDLE, &image_idx)};
    if (next_image_result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        return;
    }

    if (next_image_result != VK_SUCCESS && next_image_result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swap chain image.");
    }

    vkResetFences(logical_device, 1, &in_flight_fence[current_frame]);

    vkResetCommandBuffer(command_buffer[current_frame], 0);
    record_command_buffer(command_buffer[current_frame], image_idx);

    const Array<VkSemaphore, 1> wait_semaphores_array = {image_available_semaphore[current_frame]};
    const Array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    const Array<VkSemaphore, 1> signal_semaphores_array = {render_finished_semaphore[current_frame]};

    const VkSubmitInfo submit_info{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores_array.data(),
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer[current_frame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores_array.data(),
    };

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence[current_frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    const Array<VkSwapchainKHR, 1> swapchains_array = {swapchain};
    const VkPresentInfoKHR present_info{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores_array.data(),
        .swapchainCount = 1,
        .pSwapchains = swapchains_array.data(),
        .pImageIndices = &image_idx,
        .pResults = nullptr,
    };

    const VkResult present_result{vkQueuePresentKHR(present_queue, &present_info)};
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR)
    {
        frame_buffer_resized = false;
        recreateSwapchain();
    }
    else if (present_result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swap chain image.");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
auto Application::cBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                          const VkMemoryPropertyFlags properties, Buffer &buffer) -> void
{

    const VkBufferCreateInfo buffer_info{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                         .pNext = nullptr,
                                         .flags = 0,
                                         .size = size,
                                         .usage = usage,
                                         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                         .queueFamilyIndexCount = 0,
                                         .pQueueFamilyIndices = nullptr};

    if (vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer.first) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer.");
    }

    VkMemoryRequirements mem_reqs{};
    vkGetBufferMemoryRequirements(logical_device, buffer.first, &mem_reqs);

    const VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_mem_type(mem_reqs.memoryTypeBits, properties),
    };

    if (vkAllocateMemory(logical_device, &alloc_info, nullptr, &buffer.second) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(logical_device, buffer.first, buffer.second, 0);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
auto Application::cVertexBuffer() -> void
{
    const VkDeviceSize buffer_size{sizeof(vertices[0]) * vertices.size()};

    Buffer staging_buffer{};
    cBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer);

    void *pdata{nullptr};
    vkMapMemory(logical_device, staging_buffer.second, 0, buffer_size, 0, &pdata);

    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    memcpy(pdata, vertices.data(), reinterpret_cast<size_t>(buffer_size));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    vkUnmapMemory(logical_device, staging_buffer.second);

    cBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer);

    copy_buffer(staging_buffer.first, vertex_buffer.first, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer.first, nullptr);
    vkFreeMemory(logical_device, staging_buffer.second, nullptr);
}
auto Application::cIndexBuffer() -> void
{
    const VkDeviceSize buffer_size{sizeof(indices[0]) * indices.size()};

    Buffer staging_buffer{};
    cBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer);

    void *data{nullptr};
    vkMapMemory(logical_device, staging_buffer.second, 0, buffer_size, 0, &data);
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    memcpy(data, indices.data(), reinterpret_cast<size_t>(buffer_size));
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    vkUnmapMemory(logical_device, staging_buffer.second);

    cBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer);

    copy_buffer(staging_buffer.first, index_buffer.first, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer.first, nullptr);
    vkFreeMemory(logical_device, staging_buffer.second, nullptr);
}
auto Application::find_mem_type(const uInt32 type_filter, const VkMemoryPropertyFlags properties) const -> uInt32
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t idx = 0; idx < mem_properties.memoryTypeCount; idx++)
    {
        if (((type_filter & (1 << idx)) != 0U) &&
            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
            (mem_properties.memoryTypes[idx].propertyFlags & properties) == properties)
        // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
        {
            return idx;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type.");
}
auto Application::copy_buffer(const VkBuffer src, const VkBuffer dst, const VkDeviceSize size) const noexcept -> void
{
    const VkCommandBufferAllocateInfo alloc_info{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                                 .pNext = nullptr,
                                                 .commandPool = command_pool,
                                                 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                                 .commandBufferCount = 1};

    VkCommandBuffer copy_command_buffer{};
    vkAllocateCommandBuffers(logical_device, &alloc_info, &copy_command_buffer);

    const VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                             .pNext = nullptr,
                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                                             .pInheritanceInfo = nullptr};

    vkBeginCommandBuffer(copy_command_buffer, &beginInfo);

    const VkBufferCopy copy_region{.srcOffset = 0, .dstOffset = 0, .size = size};

    vkCmdCopyBuffer(copy_command_buffer, src, dst, 1, &copy_region);
    vkEndCommandBuffer(copy_command_buffer);

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &copy_command_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &copy_command_buffer);
}
auto Application::cDescriptorSetLayout() -> void
{
    const VkDescriptorSetLayoutBinding ubo_layout_binding{.binding = 0,
                                                          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          .descriptorCount = 1,
                                                          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                                          .pImmutableSamplers = nullptr};

    const VkDescriptorSetLayoutCreateInfo layout_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                                                      .pNext = nullptr,
                                                      .flags = 0,
                                                      .bindingCount = 1,
                                                      .pBindings = &ubo_layout_binding};

    if (vkCreateDescriptorSetLayout(logical_device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

} // namespace MyVk
