// clang-format off

#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "Vk/Application.hpp"
#include "Vk/common_includes.hpp"

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

TEST_CASE("Quick check", "[main]") {
  MyVk::Application app{};
  app.unit_test();
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
