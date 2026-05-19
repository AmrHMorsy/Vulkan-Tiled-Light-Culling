#pragma once
#include "Includes.h"
#include "Structs.h"



class Buffer {
    
public:
    
    static BufferInfo BuildBuffer(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags properties)
    {
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
        
        VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = sharingMode
        };
        if(vkCreateBuffer(logicalDevice, &bufferCreateInfo, NULL, &buffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE BUFFER");
        
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &memoryRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
            if((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties){
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
        if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");
        
        vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
        
        return BufferInfo{
            .buffer = buffer,
            .bufferMemory = bufferMemory
        };
    }
    
    static HostVisibleBuffer BuildHostVisibleBuffer(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usage)
    {
        BufferInfo bufferPack;
        VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        if(vkCreateBuffer(logicalDevice, &bufferCreateInfo, NULL, &bufferPack.buffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE BUFFER");
        
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, bufferPack.buffer, &memoryRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
            if((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties){
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
        if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferPack.bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");
        
        vkBindBufferMemory(logicalDevice, bufferPack.buffer, bufferPack.bufferMemory, 0);

        void * memoryPointer;
        vkMapMemory(logicalDevice, bufferPack.bufferMemory, 0, bufferSize, 0, &memoryPointer);
        
        return HostVisibleBuffer{
            .bufferPack = bufferPack,
            .memoryPointer = memoryPointer
        };
    }
    
    static BufferInfo BuildDataBuffer(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, void * data, VkDeviceSize bufferSize, uint32_t queueIndex)
    {
        VkMemoryAllocateInfo allocInfo{};
        VkMemoryPropertyFlags properties;
        BufferInfo stageBuffer, dataBuffer;
        VkMemoryRequirements memoryRequirements;

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
        
        VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        if(vkCreateBuffer(logicalDevice, &bufferCreateInfo, NULL, &stageBuffer.buffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE BUFFER");
        
        vkGetBufferMemoryRequirements(logicalDevice, stageBuffer.buffer, &memoryRequirements);
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
            if((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties){
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
        if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &stageBuffer.bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");
        
        vkBindBufferMemory(logicalDevice, stageBuffer.buffer, stageBuffer.bufferMemory, 0);
        
        void * memoryPointer;
        vkMapMemory(logicalDevice, stageBuffer.bufferMemory, 0, bufferSize, 0, &memoryPointer);
        memcpy(memoryPointer, data, static_cast<size_t>(bufferSize));
        vkUnmapMemory(logicalDevice, stageBuffer.bufferMemory);
        
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if(vkCreateBuffer(logicalDevice, &bufferCreateInfo, NULL, &dataBuffer.buffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE BUFFER");
        
        vkGetBufferMemoryRequirements(logicalDevice, dataBuffer.buffer, &memoryRequirements);
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
            if((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties){
                allocInfo.memoryTypeIndex = i;
                break;
            }
        }
        if(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &dataBuffer.bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");

        vkBindBufferMemory(logicalDevice, dataBuffer.buffer, dataBuffer.bufferMemory, 0);

        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = queueIndex;
        VkCommandPool commandPool;
        if(vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE COMMAND POOL\n");
        
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        if(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE COMMAND BUFFERS\n");
        
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BEGIN RECORDING COMMAND BUFFER\n");
        
        VkBufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bufferSize
        };
        vkCmdCopyBuffer(commandBuffer, stageBuffer.buffer, dataBuffer.buffer, 1, &copyRegion);

        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO END RECORDING COMMAND BUFFER\n");

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        if(vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE FENCES\n");
        
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffer;
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.signalSemaphoreCount = 0;
        info.pSignalSemaphores = nullptr;
        if(vkQueueSubmit(graphicsQueue, 1, &info, fence) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
        
        vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(logicalDevice, fence, NULL);
        vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer) ;
        vkDestroyBuffer(logicalDevice, stageBuffer.buffer, NULL);
        vkFreeMemory(logicalDevice, stageBuffer.bufferMemory, NULL);

        return dataBuffer ;
    }
};
