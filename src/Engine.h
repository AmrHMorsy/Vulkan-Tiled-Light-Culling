#pragma once
#include "Includes.h"
#include "Structs.h"



class Engine{
    
private:
    
    static VkExtent2D GetSwapChainExtent(WindowInfo& window, PhysicalDeviceInfo& physicalDevice)
    {
        uint32_t capabilitiesWidth = physicalDevice.capabilities.currentExtent.width;
        uint32_t capabilitiesHeight = physicalDevice.capabilities.currentExtent.height;
        
        if(capabilitiesWidth != std::numeric_limits<uint32_t>::max()){
            if(capabilitiesWidth != static_cast<uint32_t>(window.fbWidth) || capabilitiesHeight != static_cast<uint32_t>(window.fbHeight)){
                VkExtent2D actualExtent = {static_cast<uint32_t>(window.fbWidth), static_cast<uint32_t>(window.fbHeight)};
                actualExtent.width  = std::clamp(actualExtent.width, physicalDevice.capabilities.minImageExtent.width, physicalDevice.capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, physicalDevice.capabilities.minImageExtent.height, physicalDevice.capabilities.maxImageExtent.height);
                return actualExtent;
            }
            else
                return physicalDevice.capabilities.currentExtent;
        }
        else{
            VkExtent2D actualExtent = {static_cast<uint32_t>(window.fbWidth), static_cast<uint32_t>(window.fbHeight)};
            actualExtent.width  = std::clamp(actualExtent.width, physicalDevice.capabilities.minImageExtent.width, physicalDevice.capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, physicalDevice.capabilities.minImageExtent.height, physicalDevice.capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
    
    static PhysicalDeviceInfo PickPhysicalDevice(VkInstance vulkanInstance, VkSurfaceKHR windowSurface)
    {
        PhysicalDeviceInfo physicalDevice;
        
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, NULL);
        if(deviceCount == 0)
            throw std::runtime_error("\nFAILURE TO FIND GPUs WITH VULKAN SUPPORT");
        
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount) ;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, physicalDevices.data());
        
        for(int i = 0; i < physicalDevices.size(); i++){
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, NULL);
            if(queueFamilyCount == 0)
                continue;
            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilies.data());
            bool found = false;
            for(int j = 0; j < queueFamilies.size(); j++){
                if(queueFamilies[j].queueFlags && VK_QUEUE_GRAPHICS_BIT){
                    physicalDevice.queueIndex = j;
                    found = true;
                    break;
                }
            }
            if(!found)
                continue;
            
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevices[i], windowSurface, &physicalDevice.capabilities);
            
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], windowSurface, &formatCount, nullptr);
            if(formatCount == 0)
                continue;
            physicalDevice.formats.clear();
            physicalDevice.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevices[i], windowSurface, &formatCount, physicalDevice.formats.data());
            
            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[i], windowSurface, &presentModeCount, nullptr);
            if(presentModeCount == 0)
                continue;
            physicalDevice.presentModes.clear();
            physicalDevice.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevices[i], windowSurface, &presentModeCount, physicalDevice.presentModes.data());
            
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevices[i], &supportedFeatures);
            if(!supportedFeatures.samplerAnisotropy)
                continue;
            
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &extensionCount, NULL);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &extensionCount, availableExtensions.data());
            const std::vector<const char*> logicalDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
            std::set<std::string> requiredExtensions(logicalDeviceExtensions.begin(), logicalDeviceExtensions.end());
            for(const auto& extension : availableExtensions)
                requiredExtensions.erase(extension.extensionName);
            if(!requiredExtensions.empty())
                continue;
            
            VkBool32 isSupported = false ;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], physicalDevice.queueIndex, windowSurface, &isSupported);
            if(!isSupported)
                continue;
            
            physicalDevice.handle = physicalDevices[i] ;
            break;
        }
        
        if(physicalDevice.handle == VK_NULL_HANDLE)
            throw std::runtime_error("FAILURE TO FIND SUITABLE PHYSICAL DEVICE");
                
        return physicalDevice;
    }
    
    static VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice)
    {
        std::vector<VkFormat> candidates = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };
        
        for(int i = 0; i < candidates.size(); i++){
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &properties);
            
            if(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                return candidates[i];
        }
        
        throw std::runtime_error("\nFAILURE TO FIND SUPPORTED FORMAT");
    }
    
public:
    
    static void Build(EngineInfo& engine, WindowInfo& window)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


        window.handle = glfwCreateWindow(1500, 1000, "LightCullingDemo", NULL, NULL);
        glfwSetWindowSizeLimits(window.handle, 1500, 1000, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwGetWindowSize(window.handle, &window.windowWidth, &window.windowHeight);
        glfwGetFramebufferSize(window.handle, &window.fbWidth, &window.fbHeight);


        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensionsGLFW(glfwExtensions, glfwExtensions + glfwExtensionCount);
        extensionsGLFW.push_back("VK_KHR_get_physical_device_properties2");
        extensionsGLFW.push_back("VK_KHR_portability_enumeration");


        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Light Culling Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;


        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        instanceCreateInfo.enabledExtensionCount = (uint32_t) extensionsGLFW.size();
        instanceCreateInfo.ppEnabledExtensionNames = extensionsGLFW.data();
        instanceCreateInfo.enabledLayerCount = 0;
        vkCreateInstance(&instanceCreateInfo, nullptr, &engine.vulkanInstance);


        if(glfwCreateWindowSurface(engine.vulkanInstance, window.handle, nullptr, &engine.windowSurface) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BUILD WINDOW SURFACE");

        engine.physicalDevice = PickPhysicalDevice(engine.vulkanInstance, engine.windowSurface);
        
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(engine.physicalDevice.handle, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(engine.physicalDevice.handle, nullptr, &extensionCount, extensions.data());
        float queuePriority = 1.0f ;
        std::vector<VkDeviceQueueCreateInfo> queue;;
        for ( int i = 0 ; i < 2 ; i++ ) {
            VkDeviceQueueCreateInfo queueCreateInfo{} ;
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO ;
            queueCreateInfo.queueFamilyIndex = engine.physicalDevice.queueIndex ;
            queueCreateInfo.queueCount = 1 ;
            queueCreateInfo.pQueuePriorities = &queuePriority ;
            queue.push_back( queueCreateInfo ) ;
        }
        
        
        VkPhysicalDeviceFeatures deviceFeatures{} ;
        deviceFeatures.samplerAnisotropy = VK_TRUE ;
        deviceFeatures.tessellationShader = VK_TRUE ;
        const std::vector<const char*> logicalDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME} ;
        
        
        VkDeviceCreateInfo deviceCreateInfo{} ;
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO ;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.queueCreateInfoCount = 2 ;
        deviceCreateInfo.pQueueCreateInfos = queue.data() ;
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures ;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(logicalDeviceExtensions.size()) ;
        deviceCreateInfo.ppEnabledExtensionNames = logicalDeviceExtensions.data() ;
        deviceCreateInfo.enabledLayerCount = 0 ;
        if(vkCreateDevice(engine.physicalDevice.handle, &deviceCreateInfo, nullptr, &engine.logicalDevice)!= VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE LOGICAL DEVICE");
        
        
        vkGetDeviceQueue(engine.logicalDevice, engine.physicalDevice.queueIndex, 0, &engine.graphicsQueue);
        vkGetDeviceQueue(engine.logicalDevice, engine.physicalDevice.queueIndex, 0, &engine.presentQueue);

        engine.depthFormat = FindDepthFormat(engine.physicalDevice.handle);
        
        engine.swapChain.extent = GetSwapChainExtent(window, engine.physicalDevice);
        
        bool found = false;
        for(int i = 0; i < engine.physicalDevice.formats.size(); i++){
            VkSurfaceFormatKHR format = engine.physicalDevice.formats[i];
            if(format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                found = true;
                engine.swapChain.swapChainSurface = format;
                break;
            }
        }
        if(!found)
            engine.swapChain.swapChainSurface = engine.physicalDevice.formats[0];
        
        
        engine.swapChain.swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        
        
        engine.swapChain.imageCount = (engine.physicalDevice.capabilities.maxImageCount > 0 && (engine.physicalDevice.capabilities.minImageCount+1) > engine.physicalDevice.capabilities.maxImageCount)? engine.physicalDevice.capabilities.maxImageCount: (engine.physicalDevice.capabilities.minImageCount+1);
        
        
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = engine.windowSurface;
        createInfo.minImageCount = engine.swapChain.imageCount;
        createInfo.imageFormat = engine.swapChain.swapChainSurface.format;
        createInfo.imageColorSpace = engine.swapChain.swapChainSurface.colorSpace;
        createInfo.imageExtent = engine.swapChain.extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = engine.physicalDevice.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = engine.swapChain.swapChainPresentMode;
        createInfo.clipped = VK_TRUE;
        if(vkCreateSwapchainKHR(engine.logicalDevice, &createInfo, nullptr, &engine.swapChain.handle) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE SWAP CHAIN");
        
        
        engine.swapChain.images.resize(engine.swapChain.imageCount);
        vkGetSwapchainImagesKHR(engine.logicalDevice, engine.swapChain.handle, &engine.swapChain.imageCount, engine.swapChain.images.data());
        
        
        engine.swapChain.imageViews.resize(engine.swapChain.images.size());
        for(size_t i = 0; i < engine.swapChain.images.size(); i++){
            VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = engine.swapChain.images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = engine.swapChain.swapChainSurface.format,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY
                }
            };
            if(vkCreateImageView(engine.logicalDevice, &viewInfo, nullptr, &engine.swapChain.imageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("FAILURE TO CREATE IMAGE VIEW");
        }
        
        
        engine.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if(vkCreateSemaphore(engine.logicalDevice, &info, nullptr, &engine.imageAvailableSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE SEMAPHORES");
        }
    }
    
    static void Destroy(EngineInfo& engine)
    {
        for(size_t i = 0; i < engine.swapChain.imageViews.size(); i++)
            vkDestroyImageView(engine.logicalDevice, engine.swapChain.imageViews[i], NULL);
        
        vkDestroySwapchainKHR(engine.logicalDevice, engine.swapChain.handle, NULL);
    }
};
