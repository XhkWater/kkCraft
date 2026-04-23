#define SDL_MAIN_HANDLED
#include "App.h"

#include <stdexcept>
#include <string>

#include "Log.h"

App::App() {
  kk::log_info("App::App begin");
  initWindow();
  kk::log_info("initWindow ok");
  loadWorldData();
  kk::log_info("loadWorldData ok");
  initVulkan();
  kk::log_info("initVulkan ok");
}

App::~App() { cleanup(); }

void App::vkCheck(VkResult result, const char *message) {
  if (result != VK_SUCCESS) {
    kk::log_error(std::string("Vulkan error: ") + message + " (code=" +
                  std::to_string(static_cast<int>(result)) + ")");
    throw std::runtime_error(message);
  }
}

int runApp() {
  App app;
  app.run();
  return 0;
}
