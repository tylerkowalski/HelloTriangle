#pragma once

#include "ht_window.hpp"

namespace ht
{
class App
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    void run();

  private:
    HtWindow htWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
};
} // namespace ht
