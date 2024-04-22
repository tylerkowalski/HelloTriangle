#include "ht_window.hpp"

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
                 GLFW_NO_API);                // no openGl context creatation
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // temp

  window =
      glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

} // namespace ht