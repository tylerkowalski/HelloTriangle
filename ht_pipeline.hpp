#pragma once

#include "ht_device.hpp"

#include <string>
#include <vector>

namespace ht {
struct PipelineConfigInfo {};

class HtPipeline {
public:
  HtPipeline(HtDevice &device, const std::string &vertFilePath,
             const std::string &fragFilePath,
             const PipelineConfigInfo &configInfo);
  ~HtPipeline() {}

  HtPipeline(const HtPipeline &) = delete;
  void operator=(const HtPipeline &) = delete;

  static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                      uint32_t height);

private:
  static std::vector<char> readFile(const std::string &filePath);

  void createGraphicsPipeline(const std::string &vertFilePath,
                              const std::string &fragFilePath,
                              const PipelineConfigInfo &configInfo);

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *shaderModule);
  HtDevice &htDevice; // device outlives any pipeline, so memory-safe
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};
} // namespace ht