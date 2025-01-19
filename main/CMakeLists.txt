add_executable(main main.cpp)

target_compile_features(main PRIVATE)

target_link_libraries(main PRIVATE vk glfw glm::glm Vulkan::Vulkan)
