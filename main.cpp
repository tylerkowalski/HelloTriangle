#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <string>

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
        VkInstance instance;

        ///////////////////////////////////////////////////////////////////////////////////////////////

        // methods:
        void initWindow(){
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't want to create an openGL context
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // temporary because resizing windows needs special care

            window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        }

        void initVulkan() {
            createInstance();
        }

        void createInstance() {
            // instance represents the connection between application and Vulkan library
            // telling driver about application's requirements and capabilities

            // create instance. technically optionaly, but driver may able to use data to optimize?
            VkApplicationInfo appInfo{}; // value-initialization: built-in types set to default value, user-defined have default constructor called
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Triangle";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            // check for extension support
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());
            std::cout << "Supported Extensions:" << std::endl;
            for (const auto& extension : supportedExtensions) {
                std::cout << '\t' << extension.extensionName << std::endl;
            }
            std::cout << std::endl;

            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // find extensions glfw needs to interface with the window system
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;

            createInfo.enabledLayerCount = 0; // next 2 parameters (including this) state the validation layers?

            // check if glfw extensions are supported (not necessary since instance creation will fail anyways)
            for (int i = 0; i < glfwExtensionCount; ++i) {
                std::cout << "Searching for " << glfwExtensions[i] << std::endl;
                bool foundExtension = false;
                for (const auto& extension : supportedExtensions) {
                    if (std::string(extension.extensionName) == std::string(glfwExtensions[i])) {
                        std::cout << "found extension!" << std::endl << std::endl;
                        foundExtension = true;
                        break;
                    }
                }
                if (!foundExtension) std::cerr << "ERROR: " << glfwExtensions[i] << " not found!" << std::endl << std::endl;
            }

            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            // general pattern: pointer to struct with creation info, pointer to custom allocator, pointer to variable storing handle to new object
            if (result != VK_SUCCESS) throw std::runtime_error("failed to create instance!");


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
            vkDestroyInstance(instance, nullptr);

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