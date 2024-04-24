#include "app.hpp"

#include <array>
#include <stdexcept>

namespace ht {

App::App() {
  loadModels();
  // loadSierpinskiModel();
  createPipelineLayout();
  createPipeline();
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
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(htDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void App::createPipeline() {
  auto pipelineConfig = HtPipeline::defaultPipelineConfigInfo(
      htSwapChain.width(), htSwapChain.height());
  pipelineConfig.renderPass = htSwapChain.getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  htPipeline = std::make_unique<HtPipeline>(
      htDevice, "shaders/simple_shader.vert.spv",
      "shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
  commandBuffers.resize(
      htSwapChain.imageCount()); // 1 command buffer per frame buffer

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = htDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(htDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  for (int i = 0; i < commandBuffers.size(); ++i) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = htSwapChain.getRenderPass();
    renderPassInfo.framebuffer = htSwapChain.getFrameBuffer(i);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = htSwapChain.getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 0.1f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    htPipeline->bind(commandBuffers[i]);
    htModel->bind(commandBuffers[i]);
    htModel->draw(commandBuffers[i]);
    // sierpinskiModel->bind(commandBuffers[i]);
    // sierpinskiModel->draw(commandBuffers[i]);

    vkCmdEndRenderPass(commandBuffers[i]);
    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}
void App::drawFrame() {
  uint32_t imageIndex;
  auto result = htSwapChain.acquireNextImage(&imageIndex);

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  result = htSwapChain.submitCommandBuffers(&commandBuffers[imageIndex],
                                            &imageIndex);

  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swapchain image!");
  }
}

void App::loadModels() {
  std::vector<HtModel::Vertex> vertices{{{-1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
                                        {{0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
                                        {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}};
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

    vertices.emplace_back(HtModel::Vertex{a});
    vertices.emplace_back(HtModel::Vertex{x});
    vertices.emplace_back(HtModel::Vertex{z});
    std::vector<glm::vec2> Triangle1{a, x, z};

    vertices.emplace_back(HtModel::Vertex{x});
    vertices.emplace_back(HtModel::Vertex{b});
    vertices.emplace_back(HtModel::Vertex{y});
    std::vector<glm::vec2> Triangle2{x, b, y};

    vertices.emplace_back(HtModel::Vertex{z});
    vertices.emplace_back(HtModel::Vertex{y});
    vertices.emplace_back(HtModel::Vertex{c});
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