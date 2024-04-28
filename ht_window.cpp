#include "ht_window.hpp"

#include <stdexcept>

namespace ht {
HtWindow::HtWindow(int w, int h, std::string name)
    : width{w}, height{h}, windowName{name} {
  initWindow();
}

HtWindow::~HtWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

bool HtWindow::shouldClose() { return glfwWindowShouldClose(window); }

void HtWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API,
                 GLFW_NO_API);               // no openGl context creatation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // temp

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void HtWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void HtWindow::framebufferResizeCallback(GLFWwindow *window, int width,
                                         int height) {
  auto htWindow =
      reinterpret_cast<HtWindow *>(glfwGetWindowUserPointer(window));
  htWindow->framebufferResized = true;
  htWindow->width = width;
  htWindow->height = height;
}

} // namespace ht