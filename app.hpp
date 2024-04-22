#pragma once

#include "ht_device.hpp"
#include "ht_pipeline.hpp"
#include "ht_swap_chain.hpp"
#include "ht_window.hpp"

#include <memory>

namespace ht {
class App {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();

private:
  HtWindow htWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
  HtDevice htDevice{htWindow};
  HtSwapChain htSwapChain{htDevice, htWindow.getExtent()};
  std::unique_ptr<HtPipeline> htPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;

  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();
};
} // namespace ht
