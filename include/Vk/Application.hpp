#pragma once
#include "Vk/common_includes.hpp"

namespace MyVk
{

class Application
{
    struct AppWindow
    {
        GLFWwindow *win{};
        int width{800};
        int height{600};
    };
    struct QueueIndices
    {
        uInt32_opt graphics_indices{};
        uInt32_opt present_indice{};
    };
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        Vec<VkSurfaceFormatKHR> formats{};
        Vec<VkPresentModeKHR> present_modes{};

        auto swapchain_adequate() const noexcept -> bool
        {
            return !formats.empty() && !present_modes.empty();
        };
    };
    struct ShaderCode
    {
        Vec<char> vert_shader{readShader("shaders/vert.spv")};
        Vec<char> frag_shader{readShader("shaders/frag.spv")};
    };

  private:
    bool fullscreen{};
    AppWindow window{};
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debug_messenger{};
    ShaderCode shader_code{};
    size_t current_frame{};
    bool frame_buffer_resized{};

    auto cWindow() -> void;
    auto init_vulkan() -> void;
    auto cInstance() -> void;
    auto esc_to_quit() const noexcept -> void;
    auto clean_up() noexcept -> void;
    auto draw_frame() -> void;

    // ============ Validation Layers defined in Layers.cpp =======
    auto static check_validation_layer_support() -> bool;
    auto setup_debug_messenger() -> void;
    auto static get_required_extensions() -> Vec<cStr>;
    // clang-format off
    [[nodiscard]] auto static CreateDebugUtilsMessengerEXT(VkInstance instance,
											 const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                             VkDebugUtilsMessengerEXT *pDebugMessenger) -> VkResult;

    auto static DestroyDebugUtilsMessengerEXT(VkInstance instance,
											  VkDebugUtilsMessengerEXT debugMessenger) -> void;
   
    [[nodiscard]] auto static debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                void *pUserData) -> VKAPI_ATTR VkBool32 VKAPI_CALL;
    // clang-format on    
    auto static framebufferResizeCallback(GLFWwindow* window, int width, int height) -> void;
  
    // ============ Devices and Queues defined in Devices.cpp =======
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice logical_device{};
    VkQueue graphics_queue{};
    VkQueue present_queue{};

    auto cPhysicalDevice() -> void;
    auto cLogicalDevice() -> void;
    [[nodiscard]] auto static find_device(const Vec<VkPhysicalDevice> &devices) noexcept -> VkPhysicalDevice;
    auto check_device_exts_support() const noexcept -> bool;
    template <VkQueueFlagBits queue_flag> [[nodiscard]] auto find_queue_families() const noexcept -> QueueIndices;

    // ============ Surface and Swapchain/Images defined in Swapchain.cpp =======
    VkSurfaceKHR surface{};
    VkSwapchainKHR swapchain{};
    Vec<VkImage> swapchain_images{};
    Vec<VkImageView> swapchain_image_views{};
    VkFormat swapchain_image_format{};
    VkExtent2D swapchain_extent{};

    auto cSwapchain() -> void;
    auto recreateSwapchain() -> void;
    auto cleanup_swapchain() -> void;
    auto cImageViews() -> void;
    auto query_swapchain_support() const noexcept -> SwapChainSupportDetails;
    [[nodiscard]] auto static choose_swap_surface_format(const Vec<VkSurfaceFormatKHR> &available_formats) noexcept
        -> VkSurfaceFormatKHR;
    [[nodiscard]] auto static choose_swap_present_mode(const Vec<VkPresentModeKHR> &available_presepent_modes) noexcept
        -> VkPresentModeKHR;
    [[nodiscard]] auto choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) const noexcept -> VkExtent2D;

    // ============ GraphicsPipeline defined in Pipeline.cpp =======
    VkPipelineLayout pipeline_layout{};
    VkDescriptorSetLayout descriptor_set_layout{};
    VkRenderPass render_pass{};
    VkPipeline graphics_pipeline{};
  
    auto cGraphicsPipeline() -> void;
    [[nodiscard]] auto cShaderModule(const Vec<char> &code) const -> VkShaderModule;
    auto cRenderPass() -> void;

  // ============ Buffers and Drawing defined in Drawing.cpp =======
  Vec<VkFramebuffer> swapchain_framebuffer{};
  VkCommandPool command_pool{};
  Vec<VkCommandBuffer> command_buffer{};
  Vec<VkSemaphore> image_available_semaphore{};
  Vec<VkSemaphore> render_finished_semaphore{};
  Vec<VkFence> in_flight_fence{};
  Buffer vertex_buffer{};
  Buffer index_buffer{};
  Vec<Buffer> uniform_buffers{};

  auto cIndexBuffer() -> void;  
  auto cVertexBuffer() -> void;
  [[nodiscard]] auto find_mem_type(const uInt32 type_filter, const VkMemoryPropertyFlags properties) const -> uInt32;
  auto cFramebuffers() -> void;
  auto cCommandPool() -> void;
  auto cCommandBuffer() -> void;
  auto record_command_buffer(const VkCommandBuffer buffer, const uInt32 image_index) -> void;
  auto cBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
													 const VkMemoryPropertyFlags properties, Buffer& buffer) -> void;
  auto copy_buffer(const VkBuffer src, const VkBuffer dst, const VkDeviceSize
				   size) const noexcept -> void;
  auto cSyncObjects() -> void;
  auto cDescriptorSetLayout() -> void;

  public:
    explicit Application()
    {
        cWindow();
        init_vulkan();
    }
    explicit Application(bool is_fullscreen)
    {
	    fullscreen = is_fullscreen;
        cWindow();
        init_vulkan();
    }
    ~Application() noexcept
    {
        clean_up();
    }

    Application(const Application &other) = default;
    auto operator=(Application const &other) -> Application & = default;
    Application(Application &&other) noexcept = default;
    auto operator=(Application &&other) noexcept -> Application & = default;

    auto triangle() noexcept -> void;
    [[nodiscard]] auto static readShader(const std::string &file_name) -> Vec<char>;
};

} // namespace MyVk
