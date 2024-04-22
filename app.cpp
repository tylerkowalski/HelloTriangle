#include "app.hpp"

#include <stdexcept>

namespace ht {

App::App() {
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
  }
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

void App::createCommandBuffers() {}
void App::drawFrame() {}

} // namespace ht