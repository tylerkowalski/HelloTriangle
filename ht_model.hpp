#pragma once

#include "ht_device.hpp"

#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm assumes openGl standard, depth [-1,1]
                                    // instead of [0,1]
#include <glm/glm.hpp>

namespace ht {
class HtModel {
public:
  struct Vertex {
    glm::vec2 position;

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  HtModel(HtDevice &device, const std::vector<Vertex> &vertices);
  ~HtModel();

  HtModel(const HtModel &) = delete;
  HtModel &operator=(const HtModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  HtDevice &htDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;

  void createVertexBuffers(const std::vector<Vertex> &vertices);
};
} // namespace ht