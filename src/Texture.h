#pragma once
#include "Includes.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../Resources/external/stb_image/stb_image.h"



class Texture{
    
public:
    
    static void Build(EngineInfo& engine, TextureInfo& texture, std::string filePath)
    {
        // Loading Image
        //---------------------------------------------------------------
        std::string extension = std::filesystem::path(filePath).extension().string();
        void * image;
        VkDeviceSize imageSize;
        int channels, width, height;
        if( (extension == ".png") || (extension == ".jpg")){
            unsigned char * img = stbi_load(filePath.c_str(), &width, &height, &channels, 4);
            imageSize = sizeof(unsigned char);
            image = img;
        }
        else
            throw std::runtime_error("ERROR::FAILURE TO LOAD IMAGE::UNSUPPORTED IMAGE EXTENSION");
        imageSize *= width * height * 4;
        VkExtent2D imageResolution = VkExtent2D{
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height)
        };
        //---------------------------------------------------------------
        
        
        
        
        // CommandPool
        //---------------------------------------------------------------
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = engine.physicalDevice.queueIndex;
        VkCommandPool commandPool;
        if(vkCreateCommandPool(engine.logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE COMMAND POOL\n");
        //---------------------------------------------------------------
        

        
        // CommandBuffer
        //---------------------------------------------------------------
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        if(vkAllocateCommandBuffers(engine.logicalDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO ALLOCATE COMMAND BUFFERS\n");
        //---------------------------------------------------------------
        

        
        // Fence
        //---------------------------------------------------------------
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = 0;
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence;
        if(vkCreateFence(engine.logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO CREATE FENCES\n");
        //---------------------------------------------------------------
        
        

        
        // Image
        //---------------------------------------------------------------
        VkImageCreateInfo imageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = imageResolution.width,
            .extent.height = imageResolution.height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .flags = 0
        };
        ImageInfo imagePack;
        if(vkCreateImage(engine.logicalDevice, &imageCreateInfo, nullptr, &imagePack.image) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO CREATE IMAGE");

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(engine.physicalDevice.handle, &memoryProperties);
        VkMemoryRequirements memRequirements ;
        vkGetImageMemoryRequirements(engine.logicalDevice, imagePack.image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{} ;
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        bool found = false;
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
            if((memRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
                allocInfo.memoryTypeIndex = i;
                found = true;
                break;
            }
        }
        if(!found)
            throw std::runtime_error("\nFAILURE TO FIND IMAGE TYPE");
        if(vkAllocateMemory(engine.logicalDevice, &allocInfo, nullptr, &imagePack.imageMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");
        vkBindImageMemory(engine.logicalDevice, imagePack.image, imagePack.imageMemory, 0);
        //---------------------------------------------------------------
        
        


        
        
        // Transition Image from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        //---------------------------------------------------------------
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = imagePack.image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;


        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO BEGIN RECORDING COMMAND BUFFER\n");

        
        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        
        vkEndCommandBuffer(commandBuffer);
        
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffer;
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.signalSemaphoreCount = 0;
        info.pSignalSemaphores = nullptr;
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, fence) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
                
        vkWaitForFences(engine.logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);
        
        vkResetFences(engine.logicalDevice, 1, &fence);
        
        vkResetCommandBuffer(commandBuffer, 0);
        //---------------------------------------------------------------


        

        
        // Transfer data from image to buffer
        //---------------------------------------------------------------
        BufferInfo stageBuffer;
        VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = imageSize,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };
        if(vkCreateBuffer(engine.logicalDevice, &bufferCreateInfo, NULL, &stageBuffer.buffer) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO CREATE BUFFER");


        VkMemoryRequirements memoryRequirements ;
        vkGetBufferMemoryRequirements(engine.logicalDevice, stageBuffer.buffer, &memoryRequirements);
        

        VkPhysicalDeviceMemoryProperties memoryProperties2;
        vkGetPhysicalDeviceMemoryProperties(engine.physicalDevice.handle, &memoryProperties2);
        VkMemoryPropertyFlags memoryPropertiesFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        VkMemoryAllocateInfo allocInfo2{} ;
        allocInfo2.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo2.allocationSize = memoryRequirements.size;
        found = false;
        for( uint32_t i = 0 ; i < memoryProperties.memoryTypeCount; i++ ){
            if((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties2.memoryTypes[i].propertyFlags & memoryPropertiesFlags) == memoryPropertiesFlags){
                allocInfo2.memoryTypeIndex = i;
                found = true;
                break;
            }
        }
        if(!found)
            throw std::runtime_error("\nFAILURE TO FIND IMAGE TYPE");
        

        if(vkAllocateMemory(engine.logicalDevice, &allocInfo2, nullptr, &stageBuffer.bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE BUFFER MEMORY ");
        

        vkBindBufferMemory(engine.logicalDevice, stageBuffer.buffer, stageBuffer.bufferMemory, 0);
        
        
        void * memoryPointer ;
        vkMapMemory(engine.logicalDevice, stageBuffer.bufferMemory, 0, imageSize, 0, &memoryPointer);
        memcpy(memoryPointer, image, static_cast<size_t>(imageSize));
        vkUnmapMemory(engine.logicalDevice, stageBuffer.bufferMemory);
        
        
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO BEGIN RECORDING COMMAND BUFFER\n");


        VkBufferImageCopy copyRegion{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageOffset = {0, 0, 0},
            .imageSubresource.mipLevel = 0,
            .imageSubresource.layerCount = 1,
            .imageSubresource.baseArrayLayer = 0,
            .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .imageExtent = {imageResolution.width, imageResolution.height, 1},
        };
        vkCmdCopyBufferToImage(commandBuffer, stageBuffer.buffer, imagePack.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);
        

        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffer;
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.signalSemaphoreCount = 0;
        info.pSignalSemaphores = nullptr;
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, fence) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
        

        vkWaitForFences(engine.logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);


        vkResetFences(engine.logicalDevice, 1, &fence);
        
        vkResetCommandBuffer(commandBuffer, 0);
        //---------------------------------------------------------------
        



        // Transition image from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        //---------------------------------------------------------------
        srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = imagePack.image;
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.levelCount = 1;
        imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO BEGIN RECORDING COMMAND BUFFER\n");

        vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        
        vkEndCommandBuffer(commandBuffer);
                
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffer;
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.signalSemaphoreCount = 0;
        info.pSignalSemaphores = nullptr;
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, fence) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
              
        vkWaitForFences(engine.logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX);

        vkResetFences(engine.logicalDevice, 1, &fence);
        
        vkResetCommandBuffer(commandBuffer, 0);
        //---------------------------------------------------------------
        
        

        
        
        //---------------------------------------------------------------
        VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = imagePack.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_SRGB,
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
        if(vkCreateImageView(engine.logicalDevice, &viewInfo, nullptr, &texture.imageView) != VK_SUCCESS)
            throw std::runtime_error("FAILURE TO CREATE IMAGE VIEW");
        //---------------------------------------------------------------
        
        

        
        //---------------------------------------------------------------
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(engine.physicalDevice.handle, &physicalDeviceProperties);
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0;
        samplerCreateInfo.maxLod = 0.0f;
        if(vkCreateSampler(engine.logicalDevice, &samplerCreateInfo, nullptr, &texture.sampler) != VK_SUCCESS)
            throw std::runtime_error("ERROR::FAILURE TO CREATE SAMPLER\n");
        //---------------------------------------------------------------
        
        

        vkDestroyFence(engine.logicalDevice, fence, NULL);
        
        vkFreeCommandBuffers(engine.logicalDevice, commandPool, 1, &commandBuffer);
        
        vkDestroyCommandPool(engine.logicalDevice, commandPool, nullptr);
    }
};
