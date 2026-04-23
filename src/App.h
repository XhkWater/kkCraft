#pragma once

#include <array>
#include <string>
#include <vector>

#include <sdl2/SDL.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class App {
public:
  struct Vertex {
    float position[3];
    float color[3];
    float texCoord[2];
  };

  struct BlockInstance {
    glm::vec3 position;
  };

  App();
  ~App();

  void run();

private:
  std::vector<Vertex> vertices;
  std::vector<BlockInstance> blocks;

  std::string texturePath; // Path to the texture to load

  SDL_Window *window = nullptr;
  VkInstance instance = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkQueue presentQueue = VK_NULL_HANDLE;
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D swapchainExtent{};
  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  VkRenderPass renderPass = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkPipeline graphicsPipeline = VK_NULL_HANDLE;
  VkImage depthImage = VK_NULL_HANDLE;
  VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
  VkImageView depthImageView = VK_NULL_HANDLE;

  VkImage textureImage = VK_NULL_HANDLE;
  VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
  VkImageView textureImageView = VK_NULL_HANDLE;
  VkSampler textureSampler = VK_NULL_HANDLE;

  VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> descriptorSets;

  VkBuffer vertexBuffer = VK_NULL_HANDLE;
  VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> framebuffers;
  VkCommandPool commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> commandBuffers;
  VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
  VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
  VkFence inFlightFence = VK_NULL_HANDLE;
  uint32_t graphicsQueueFamily = 0;
  uint32_t presentQueueFamily = 0;
  bool framebufferResized = false;
  float renderAspectRatio = 4.0f / 3.0f;
  glm::vec3 cameraPosition = glm::vec3(1.8f, 1.5f, 2.4f);
  glm::vec3 cameraFront = glm::normalize(glm::vec3(-1.8f, -1.5f, -2.4f));
  glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
  float cameraMoveSpeed = 2.0f;
  float cameraYaw = -126.87f;
  float cameraPitch = -26.70f;
  float mouseSensitivity = 0.12f;
  bool mouseCaptured = true;

  static void vkCheck(VkResult result, const char *message);
  static std::vector<char> readBinaryFile(const std::string &path);
  void loadWorldData();

  void initWindow();
  void initVulkan();
  void cleanup();
  void cleanupSwapchain();
  void recreateSwapchain();
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) const;
  VkFormat findDepthFormat() const;
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties) const;
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  bool findQueueFamilies(VkPhysicalDevice candidate);
  void createLogicalDevice();
  void createSwapchain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createDepthResources();
  void createTextureImage();
  void createTextureImageView();
  void createTextureSampler();
  VkShaderModule createShaderModule(const std::vector<char> &code) const;
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createDescriptorPool();
  void createDescriptorSets();
  void createCommandBuffers();
  void createVertexBuffer();
  void createSyncObjects();
  void recordCommandBuffer(VkCommandBuffer commandBuffer,
                           uint32_t imageIndex) const;
  void drawFrame();

  void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
                   VkDeviceMemory &imageMemory);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                    VkBuffer &buffer, VkDeviceMemory &bufferMemory);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

int runApp();
