#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>


auto main() -> int {
  try {
    return EXIT_SUCCESS;
  } catch (const std::runtime_error &err) {
    std::cerr << "Runtime Error:  " << err.what() << "\n";
    return EXIT_FAILURE;
  } catch (std::exception &err) {
    std::cerr << "Unknown Error:  " << err.what() << "\n";
    return EXIT_FAILURE;
  }
}
