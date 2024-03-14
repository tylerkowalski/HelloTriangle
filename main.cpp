#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangleApp {
    public:
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup(); // temporary: better practice is to use RAII
        }

    private:
        // fields:
        GLFWwindow* window;

        // methods:
        void initWindow(){
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't want to create an openGL context
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // temporary because resizing windows needs special care

            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }

        void initVulkan() {

        }
        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                    break;
                }
            }
        }
        void cleanup() {
            glfwDestroyWindow(window);

            glfwTerminate();
        }
};

int main() {
    HelloTriangleApp app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}