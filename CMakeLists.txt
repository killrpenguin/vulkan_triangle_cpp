cmake_minimum_required(VERSION 3.16...3.31)

project(
  VkTriangle
  VERSION 0.2
  DESCRIPTION "Vulkan triangle from the book."
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  set(CMAKE_CXX_COMPILER "/usr/lib/llvm/19/bin/clang++")
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
  set(CMAKE_CXX_EXTENSIONS OFF)

#  include(CTest)

endif()

find_package(glfw3 REQUIRED)

find_package(glm REQUIRED)

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

include(FetchContent)

add_subdirectory(src)

add_subdirectory(main)

# if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
#  AND BUILD_TESTING)
#  add_subdirectory(tests)
# endif()

