#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

bool checkValidationLayerSupport() {
    std::cout << std::endl;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *requestedLayer : validationLayers)
    {
        bool layerFound = false;
        std::cout << "Searching for: " << requestedLayer << std::endl;
        for (const auto &availableLayer : availableLayers)
        {
            if (strcmp(requestedLayer, availableLayer.layerName) == 0)
            {
                std::cout << "Layer found!" << std::endl;
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
        {
            std::cerr << "Layer: " << requestedLayer << " not found!" << std::endl;
            return false;
        }
    }
    std::cout << std::endl;
    return true;
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

// doesn't throw error if needed queues not found (will have no value in std::optional<>)
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        ++i;
    }

    return indices;
}

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else   
    const bool enableValidationLayers = true;
#endif

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
        VkInstance instance; // initial connection between application and driver (vulkan library initialization)
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // graphics device selected. handle implictly destroyed in VkInstance destruction
        VkDevice device; // logical device to interface the physicalDevice
        VkQueue graphicsQueue; // created automatically with logical device creation

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
            pickPhysicalDevice();
            createLogicalDevice();
        }

        void createInstance() {
            // instance represents the connection between application and Vulkan library
            // telling driver about application's requirements and capabilities

            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

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

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;
            }

            // find extensions glfw needs to interface with the window system
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            createInfo.enabledExtensionCount = glfwExtensionCount;
            createInfo.ppEnabledExtensionNames = glfwExtensions;

            // check if glfw extensions are supported (not necessary since instance creation will fail anyways)
            for (int i = 0; i < glfwExtensionCount; ++i) {
                std::cout << "Searching for " << glfwExtensions[i] << std::endl;
                bool foundExtension = false;
                for (const auto& extension : supportedExtensions) {
                    if (strcmp(extension.extensionName, glfwExtensions[i]) == 0) {
                        std::cout << "Extension found!" << std::endl << std::endl;
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

        // only allow discrete graphics cards that support the queue families we want
        bool isDeviceSuitable(VkPhysicalDevice device) {
            VkPhysicalDeviceProperties deviceProperties; // e.g name, type, and supported Vulkan version
            VkPhysicalDeviceFeatures deviceFeatures; // optional features like texture compression

            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && findQueueFamilies(device).isComplete()) return true;

            return false;
        }

        // pick if discrete GPU and supported needed queues
        void pickPhysicalDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            for (const auto& device : devices) {
                if (isDeviceSuitable(device)) {
                    physicalDevice = device;
                    VkPhysicalDeviceProperties selectedDevice;
                    vkGetPhysicalDeviceProperties(physicalDevice, &selectedDevice);
                    std::cout << "GPU selected: " << selectedDevice.deviceName << std::endl;
                    break;
                }
            }
            
            if (physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }

        // queues to be created, physicalDevice features, device-specific extensions
        void createLogicalDevice() {
            // specify the queues to be created
            QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

            VkDeviceQueueCreateInfo queueCreateInfo{}; // value-initialization
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
            queueCreateInfo.queueCount = 1;
            float queuePriority = 1.0f;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{}; // need to specify used physical device features

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = 0; // device specific extensions, e.g VK_KHR_SWAPCHAIN, VK_NV_ray_tracing
            createInfo.ppEnabledExtensionNames = nullptr;
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
                createInfo.ppEnabledLayerNames = nullptr;
            }

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            // retrieve queue handle
            vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue); // at queue index 0 since only 1 queue created
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
            vkDestroyDevice(device, nullptr); // logical devices don't interact directly with instances (which is why instance isn't a parameter)

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

// validation layers prev. were instance and device specific. Previously, device specific applied to GPU-specific calls. 
// now, instance layers apply to all calls. still recommend to enable validation layers at device level for compatability

// validation layers intercept Vulkan API calls and perform various checks and validations

// you can create all of the command buffers on multiple threads and then submit them all at once on the main thread with a single low-overhead call.