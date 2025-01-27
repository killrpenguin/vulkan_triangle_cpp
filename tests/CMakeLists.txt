find_package(Catch2 3 REQUIRED)

add_executable(testlib testlib.cpp)

target_link_libraries(testlib PRIVATE vk Catch2::Catch2WithMain glfw glm::glm Vulkan::Vulkan)
