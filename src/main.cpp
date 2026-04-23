#define SDL_MAIN_HANDLED
#include "App.h"

#include <iostream>
#include <stdexcept>

#include "Log.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main() {
  try {
    kk::log_info("kkCraft starting");
    return runApp();
  } catch (const std::exception &e) {
    kk::log_error(std::string("Fatal exception: ") + e.what());
    std::cerr << e.what() << '\n';
#ifdef _WIN32
    MessageBoxA(nullptr, e.what(), "kkCraft crashed", MB_OK | MB_ICONERROR);
#endif
    return 1;
  }
}