set(HEADER_LIST "${VkTriangle_SOURCE_DIR}/include/Vk/Application.hpp"
		"${VkTriangle_SOURCE_DIR}/include/Vk/common_includes.hpp")

set(SRC_LIST "${VkTriangle_SOURCE_DIR}/src/Application.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Layers.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Devices.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/SwapChain.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Pipeline.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Drawing.cpp"
	     "${VkTriangle_SOURCE_DIR}/src/Types.cpp")

add_library(vk ${SRC_LIST} ${HEADER_LIST})

target_include_directories(vk PUBLIC ../include glm)

target_compile_features(vk PUBLIC)
