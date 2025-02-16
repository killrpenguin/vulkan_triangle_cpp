#include "Vk/Application.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

auto main([[maybe_unused]] const int argc, [[maybe_unused]] const char *argv[]) -> int
{
    try
    {
        MyVk::Application app{};
        if (argc > 1)
        {
        }
        else
        {
            app.triangle();
        }

        return EXIT_SUCCESS;
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << "Runtime Error:  " << err.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (std::exception &err)
    {
        std::cerr << "Unknown Error:  " << err.what() << "\n";
        return EXIT_FAILURE;
    }
}
