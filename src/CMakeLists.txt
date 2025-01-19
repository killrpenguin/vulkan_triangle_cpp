set(HEADER_LIST "${VkTriangle_SOURCE_DIR}/include/Vk/Instance.hpp")

set(SRC_LIST "${VkTriangle_SOURCE_DIR}/src/Instance.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Application.cpp")

add_library(vk ${SRC_LIST} ${HEADER_LIST})

target_include_directories(vk PUBLIC ../include glm)

target_compile_features(vk PUBLIC)
