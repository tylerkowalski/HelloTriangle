#pragma once

#include "ht_device.hpp"
#include "ht_model.hpp"
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
  std::unique_ptr<HtSwapChain> htSwapChain;
  std::unique_ptr<HtPipeline> htPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;

  std::unique_ptr<HtModel> htModel;
  std::unique_ptr<HtModel> sierpinskiModel;

  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void loadModels();
  void loadSierpinskiModel();
  void recreateSwapChain();
  void recordCommandBuffer(int imageIndex);
};
} // namespace ht
