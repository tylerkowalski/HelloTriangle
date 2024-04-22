#include "ht_pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace ht {
HtPipeline::HtPipeline(HtDevice &device, const std::string &vertFilePath,
                       const std::string &fragFilePath,
                       const PipelineConfigInfo &configInfo)
    : htDevice{device} {
  createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
}

std::vector<char> HtPipeline::readFile(const std::string &filePath) {
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file: " + filePath);
  }

  size_t fileSize = static_cast<size_t>(file.tellg());

  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

void HtPipeline::createGraphicsPipeline(const std::string &vertFilePath,
                                        const std::string &fragFilePath,
                                        const PipelineConfigInfo &configInfo) {
  auto vertCode = readFile(vertFilePath);
  auto fragCode = readFile(fragFilePath);

  std::cout << "vertex shader code size: " << vertCode.size() << std::endl;
  std::cout << "fragment shader code size: " << fragCode.size() << std::endl;
}

void HtPipeline::createShaderModule(const std::vector<char> &code,
                                    VkShaderModule *shaderModule) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(
      code.data()); // can do because vec allocator ensures char array satisfies
                    // worst-case alignment

  if (vkCreateShaderModule(htDevice.device(), &createInfo, nullptr,
                           shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }
}

PipelineConfigInfo HtPipeline::defaultPipelineConfigInfo(uint32_t width,
                                                         uint32_t height) {
  PipelineConfigInfo configInfo{};
  return configInfo;
}

} // namespace ht