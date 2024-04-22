#include "app.hpp"

namespace ht
{
void App::run()
{
    while (!htWindow.shouldClose())
    {
        glfwPollEvents();
    }
}
} // namespace ht