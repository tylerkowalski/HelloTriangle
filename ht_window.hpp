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

  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

private:
  const int width;
  const int height;

  std::string windowName;

  GLFWwindow *window;

  void initWindow();
};
} // namespace ht