#include "App.h"

#include <algorithm>
#include <cmath>
#include <string>

void App::run() {
  bool running = true;
  uint64_t previousTicks = SDL_GetPerformanceCounter();
  float fpsTimer = 0.0f;
  uint32_t fpsFrames = 0;

  while (running) {
    const uint64_t currentTicks = SDL_GetPerformanceCounter();
    const float deltaSeconds = static_cast<float>(currentTicks - previousTicks) /
                               static_cast<float>(SDL_GetPerformanceFrequency());
    previousTicks = currentTicks;
    fpsTimer += deltaSeconds;
    fpsFrames += 1;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        mouseCaptured = !mouseCaptured;
        SDL_SetRelativeMouseMode(mouseCaptured ? SDL_TRUE : SDL_FALSE);
        SDL_ShowCursor(mouseCaptured ? SDL_DISABLE : SDL_ENABLE);
      } else if (event.type == SDL_MOUSEMOTION && mouseCaptured) {
        cameraYaw += static_cast<float>(event.motion.xrel) * mouseSensitivity;
        cameraPitch -= static_cast<float>(event.motion.yrel) * mouseSensitivity;
        cameraPitch = std::clamp(cameraPitch, -89.0f, 89.0f);
      } else if (event.type == SDL_WINDOWEVENT &&
                 event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        framebufferResized = true;
      }
    }

    glm::vec3 front{};
    front.x = std::cos(glm::radians(cameraYaw)) * std::cos(glm::radians(cameraPitch));
    front.y = std::sin(glm::radians(cameraPitch));
    front.z = std::sin(glm::radians(cameraYaw)) * std::cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(front);

    const Uint8 *keys = SDL_GetKeyboardState(nullptr);
    const glm::vec3 forward = glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
    const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
    glm::vec3 movement(0.0f);

    if (keys[SDL_SCANCODE_W]) movement += forward;
    if (keys[SDL_SCANCODE_S]) movement -= forward;
    if (keys[SDL_SCANCODE_D]) movement += right;
    if (keys[SDL_SCANCODE_A]) movement -= right;
    if (keys[SDL_SCANCODE_E]) movement += worldUp;
    if (keys[SDL_SCANCODE_Q]) movement -= worldUp;

    if (glm::length(movement) > 0.0f) {
      movement = glm::normalize(movement) * cameraMoveSpeed * deltaSeconds;
      cameraPosition += movement;
    }

    if (fpsTimer >= 1.0f) {
      const uint32_t fps = static_cast<uint32_t>(static_cast<float>(fpsFrames) / fpsTimer);
      SDL_SetWindowTitle(window, ("kkCraft-----FPS:" + std::to_string(fps)).c_str());
      fpsTimer = 0.0f;
      fpsFrames = 0;
    }

    drawFrame();
  }
  vkDeviceWaitIdle(device);
}
