#define SDL_MAIN_HANDLED
#include "App.h"

#include <iostream>
#include <stdexcept>

int main() {
  try {
    return runApp();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}