#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace ht {

class HtWindow {
public:
  HtWindow(int w, int h, std::string name);
  ~HtWindow();

  HtWindow(const HtWindow &) = delete;
  HtWindow &operator=(const HtWindow &) = delete;

  bool shouldClose();

  VkExtent2D getExtent() {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }

  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;

  GLFWwindow *window;

  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height);
  void initWindow();
};
} // namespace ht