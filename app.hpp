#pragma once

#include "ht_device.hpp"
#include "ht_pipeline.hpp"
#include "ht_window.hpp"

namespace ht {
class App {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  void run();

private:
  HtWindow htWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
  HtDevice htDevice{htWindow};
  HtPipeline htPipeline{htDevice, "shaders/simple_shader.vert.spv",
                        "shaders/simple_shader.frag.spv",
                        HtPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
};
} // namespace ht
