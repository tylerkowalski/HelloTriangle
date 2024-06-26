#include "app.hpp"

#include <array>
#include <cassert>
#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm assumes openGl standard, depth [-1,1]
                                    // instead of [0,1]
#include <glm/glm.hpp>

namespace ht {

// todo: move push constant structure out of application implementation
struct SimplePushConstantData {
  glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

App::App() {
  loadModels();
  // loadSierpinskiModel();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}
App::~App() {
  vkDestroyPipelineLayout(htDevice.device(), pipelineLayout, nullptr);
}

void App::run() {
  while (!htWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }
  vkDeviceWaitIdle(htDevice.device());
}

void App::createPipelineLayout() {

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(htDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void App::createPipeline() {
  assert(htSwapChain != nullptr && "Cannot create pipline before swap chain!");
  assert(pipelineLayout != nullptr &&
         "Cannot create pipline before pipeline layout!");

  PipelineConfigInfo pipelineConfig{};

  HtPipeline::defaultPipelineConfigInfo(pipelineConfig);

  pipelineConfig.renderPass = htSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  htPipeline = std::make_unique<HtPipeline>(
      htDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::recreateSwapChain() {
  auto extent = htWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = htWindow.getExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(htDevice.device());

  if (htSwapChain == nullptr) {
    htSwapChain = std::make_unique<HtSwapChain>(htDevice, extent);
  } else {
    htSwapChain =
        std::make_unique<HtSwapChain>(htDevice, extent, std::move(htSwapChain));
    if (htSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  // note that if the previous renderpass is compatible, we do not need to
  // create a new pipeline
  createPipeline();
}

void App::createCommandBuffers() {
  commandBuffers.resize(
      htSwapChain->imageCount()); // 1 command buffer per frame buffer

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = htDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(htDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void App::freeCommandBuffers() {
  vkFreeCommandBuffers(htDevice.device(), htDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void App::recordCommandBuffer(int imageIndex) {
  static int frame = 0;
  frame = (frame + 1) % 100;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = htSwapChain->getRenderPass();
  renderPassInfo.framebuffer = htSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = htSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(htSwapChain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(htSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 0.0f;
  VkRect2D scissor{{0, 0}, htSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  htPipeline->bind(commandBuffers[imageIndex]);
  htModel->bind(commandBuffers[imageIndex]);
  // htModel->draw(commandBuffers[i]);
  // sierpinskiModel->bind(commandBuffers[imageIndex]);

  for (int i = 0; i < 4; i++) {
    SimplePushConstantData push{};
    push.offset = {-0.5f + frame * 0.05f, -0.4f + i * 0.25f};
    push.color = {0.0f, 0.0f, 0.2f + 0.2f * i};

    vkCmdPushConstants(commandBuffers[imageIndex], pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SimplePushConstantData), &push);
    htModel->draw(commandBuffers[imageIndex]);
  }

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void App::drawFrame() {
  uint32_t imageIndex;
  auto result = htSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  recordCommandBuffer(imageIndex);
  result = htSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                             &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      htWindow.wasWindowResized()) {
    htWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swapchain image!");
  }
}

void App::loadModels() {
  std::vector<HtModel::Vertex> vertices{{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
  htModel = std::make_unique<HtModel>(htDevice, vertices);
}

void recursiveGen(std::vector<HtModel::Vertex> &vertices,
                  std::vector<glm::vec2> curTriangle, int level) {
  if (level < 2) {
    glm::vec2 a = curTriangle[0];
    glm::vec2 b = curTriangle[1];
    glm::vec2 c = curTriangle[2];

    glm::vec2 x = 0.5f * (a + b);
    glm::vec2 y = 0.5f * (b + c);
    glm::vec2 z = 0.5f * (a + c);

    vertices.emplace_back(HtModel::Vertex{a, {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{x, {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{z, {0.0f, 0.0f, 1.0f}});
    std::vector<glm::vec2> Triangle1{a, x, z};

    vertices.emplace_back(HtModel::Vertex{x, {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{b, {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{y, {0.0f, 0.0f, 1.0f}});
    std::vector<glm::vec2> Triangle2{x, b, y};

    vertices.emplace_back(HtModel::Vertex{z, {1.0f, 0.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{y, {0.0f, 1.0f, 0.0f}});
    vertices.emplace_back(HtModel::Vertex{c, {0.0f, 0.0f, 1.0f}});
    std::vector<glm::vec2> Triangle3{z, y, c};

    recursiveGen(vertices, Triangle1, level + 1);
    recursiveGen(vertices, Triangle2, level + 1);
    recursiveGen(vertices, Triangle3, level + 1);
  }
}

void App::loadSierpinskiModel() {
  // std::vector<HtModel::Vertex> modelVertices{
  //     {{-1.0f, 1.0f}}, {{0.0f, -1.0f}}, {{1.0f, 1.0f}}};
  std::vector<HtModel::Vertex> modelVertices{};
  std::vector<glm::vec2> vertices{{-1.0f, 1.0f}, {0.0f, -1.0f}, {1.0f, 1.0f}};

  recursiveGen(modelVertices, vertices, 1);
  sierpinskiModel = std::make_unique<HtModel>(htDevice, modelVertices);
}

} // namespace ht