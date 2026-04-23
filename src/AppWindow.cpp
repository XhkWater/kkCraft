#include "App.h"

#include "Log.h"

#ifdef _WIN32
#include <dwmapi.h>
#include <sdl2/SDL_syswm.h>
#include <windows.h>
#endif

#include <stdexcept>

#ifdef _WIN32
static void enableDarkTitleBar(SDL_Window *window) {
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
    return;
  }

  HWND hwnd = wmInfo.info.win.window;
  if (!hwnd) {
    return;
  }

  BOOL enabled = TRUE;
  constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE = 20;
  constexpr DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_OLD = 19;
  HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE,
                                     &enabled, sizeof(enabled));
  if (FAILED(hr)) {
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE_OLD, &enabled,
                          sizeof(enabled));
  }
}
#endif

void App::initWindow() {
  kk::log_info("initWindow: SDL_Init");
  SDL_Init(SDL_INIT_VIDEO);
  kk::log_info("initWindow: SDL_CreateWindow");
  window = SDL_CreateWindow("kkCraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_MAXIMIZED);
#ifdef _WIN32
  if (window) {
    enableDarkTitleBar(window);
  }
#endif
  if (!window) {
    kk::log_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    throw std::runtime_error("Failed to create SDL window");
  }

  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_ShowCursor(SDL_DISABLE);

  int width = 0;
  int height = 0;
  SDL_GetWindowSize(window, &width, &height);
  if (width > 0 && height > 0) {
    renderAspectRatio = static_cast<float>(width) / static_cast<float>(height);
  }
}
