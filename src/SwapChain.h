#pragma once
#include "Structs.h"
#include "Includes.h"



class SwapChain{
    
public:
    
    static uint32_t AcquireNextImage(EngineInfo& engine, WindowInfo& window, uint32_t frame)
    {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(engine.logicalDevice, engine.swapChain.handle, UINT64_MAX, engine.imageAvailableSemaphores[frame], VK_NULL_HANDLE, &imageIndex);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            imageIndex = -1;
        else if(result != VK_SUCCESS)
            throw std::runtime_error("Failed to Acquire Swap Chain Image");
        
        return imageIndex;
    }

    static bool PresentImage(EngineInfo& engine, VkSemaphore& waitSemaphore)
    {
        std::vector<VkSemaphore> waitSemaphores = {waitSemaphore};
        std::vector<VkSwapchainKHR> swapChains = {engine.swapChain.handle};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
        presentInfo.pWaitSemaphores = waitSemaphores.data();
        presentInfo.swapchainCount = static_cast<uint32_t>(swapChains.size());
        presentInfo.pSwapchains = swapChains.data();
        presentInfo.pImageIndices = &engine.imageIndex;
        VkResult result = vkQueuePresentKHR(engine.presentQueue, &presentInfo);
        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            return false;
        else if(result != VK_SUCCESS)
            throw std::runtime_error( "Failed To present Swap Chain Image ");
        
        return true;
    }
};
