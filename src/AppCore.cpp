#define SDL_MAIN_HANDLED
#include "App.h"

#include <stdexcept>

App::App() {
  initWindow();
  initVulkan();
}

App::~App() { cleanup(); }

void App::vkCheck(VkResult result, const char *message) {
  if (result != VK_SUCCESS) {
    throw std::runtime_error(message);
  }
}

void App::initVulkan() {
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapchain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createDepthResources();
  createFramebuffers();
  createCommandPool();
  createVertexBuffer();
  createCommandBuffers();
  createSyncObjects();
}

void App::cleanup() {
  if (device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
  }
  if (inFlightFence != VK_NULL_HANDLE) {
    vkDestroyFence(device, inFlightFence, nullptr);
  }
  if (renderFinishedSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
  }
  if (imageAvailableSemaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
  }

  cleanupSwapchain();

  if (vertexBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device, vertexBuffer, nullptr);
  }
  if (vertexBufferMemory != VK_NULL_HANDLE) {
    vkFreeMemory(device, vertexBufferMemory, nullptr);
  }
  if (commandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
  }
  if (device != VK_NULL_HANDLE) {
    vkDestroyDevice(device, nullptr);
  }
  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
  }
  if (instance != VK_NULL_HANDLE) {
    vkDestroyInstance(instance, nullptr);
  }
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
  SDL_Quit();
}

int runApp() {
  App app;
  app.run();
  return 0;
}
