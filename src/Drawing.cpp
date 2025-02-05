#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"
#include <vulkan/vulkan_core.h>

namespace MyVk
{
auto Application::cFramebuffers() -> void
{
    swapchain_framebuffer.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++)
    {
        Array<VkImageView, 1> attachments = {swapchain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = swapchain_extent.width;
        framebuffer_info.height = swapchain_extent.height;
        framebuffer_info.layers = 1;

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

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices[0];

    if (vkCreateCommandPool(logical_device, &pool_info, nullptr, &command_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool.");
    }
}
auto Application::cCommandBuffer() -> void
{
    command_buffer.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uInt32>(command_buffer.size());

    if (vkAllocateCommandBuffers(logical_device, &alloc_info, command_buffer.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}
auto Application::record_command_buffer(const VkCommandBuffer buffer, const uInt32 image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;                  // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer.");
    }

    VkClearValue clear_color = {{{0.0F, 0.0F, 0.0F, 1.0F}}};
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = swapchain_framebuffer[image_index];
    render_pass_info.renderArea.offset = {.x = 0, .y = 0};
    render_pass_info.renderArea.extent = swapchain_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    VkViewport viewport{};
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = static_cast<float>(swapchain_extent.width);
    viewport.height = static_cast<float>(swapchain_extent.height);
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {.x = 0, .y = 0};
    scissor.extent = swapchain_extent;
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    Array<VkBuffer, 1> vertex_buffers_array{vertex_buffer.first};
    Array<VkDeviceSize, 1> offsets_array{0};

    vkCmdBindVertexBuffers(buffer, 0, 1, vertex_buffers_array.data(), offsets_array.data());
    vkCmdDraw(buffer, vertices.size(), 1, 0, 0);
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

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

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

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    Array<VkSemaphore, 1> wait_semaphores_array = {image_available_semaphore[current_frame]};
    Array<VkPipelineStageFlags, 1> wait_stages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores_array.data();
    submit_info.pWaitDstStageMask = wait_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer[current_frame];

    Array<VkSemaphore, 1> signal_semaphores_array = {render_finished_semaphore[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores_array.data();

    if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence[current_frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR present_info{};
    Array<VkSwapchainKHR, 1> swapchains_array = {swapchain};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores_array.data();
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains_array.data();
    present_info.pImageIndices = &image_idx;

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
auto Application::cBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties,
                          Buffer &buffer) -> void
{

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(logical_device, &buffer_info, nullptr, &buffer.first) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer.");
    }
	
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(logical_device, buffer.first, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_mem_type(mem_reqs.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &alloc_info, nullptr, &buffer.second) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(logical_device, buffer.first, buffer.second, 0);
}
// NOLINTEND(bugprone-easily-swappable-parameters)
auto Application::cVertexBuffer() -> void
{
    const VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

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
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer copy_command_buffer{};
    vkAllocateCommandBuffers(logical_device, &alloc_info, &copy_command_buffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(copy_command_buffer, &beginInfo);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0; // Optional
    copy_region.dstOffset = 0; // Optional
    copy_region.size = size;

    vkCmdCopyBuffer(copy_command_buffer, src, dst, 1, &copy_region);
    vkEndCommandBuffer(copy_command_buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &copy_command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &copy_command_buffer);
}

} // namespace MyVk
