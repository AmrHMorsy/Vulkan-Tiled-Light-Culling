#pragma once
#include "Includes.h"
#include "Engine.h"
#include "Buffer.h"




class LightCullingPrepass{

public:
    
    static void Build(EngineInfo& engine, DepthPrepassInfo& depthPrepass, LightCullingPrepassInfo& lightCullingPrepass)
    {
        lightCullingPrepass.numCellsPerTile = 16;
        lightCullingPrepass.numTiles2D.x = static_cast<unsigned int>(std::ceil(engine.swapChain.extent.width/tileResolution.x));
        lightCullingPrepass.numTiles2D.y = static_cast<unsigned int>(std::ceil(engine.swapChain.extent.height/tileResolution.y));
        lightCullingPrepass.uniformData.numTiles2D = lightCullingPrepass.numTiles2D;
        lightCullingPrepass.totalNumTiles = lightCullingPrepass.numTiles2D.x * lightCullingPrepass.numTiles2D.y;


        
        std::vector<VkShaderModule> shaderModules(1);
        std::vector<uint32_t> shaderData;
        std::ifstream file("../Resources/Shaders/LightCulling/LightCullingPrepass.spv", std::ios::binary | std::ios::ate ) ;
        if(!file.is_open())
            throw std::runtime_error("FAILURE TO OPEN SHADER FILE FOR READING");
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        shaderData.resize(fileSize / sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(shaderData.data()), fileSize);
        
        
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderData.size() * sizeof(uint32_t);
        createInfo.pCode = shaderData.data();
        if(vkCreateShaderModule(engine.logicalDevice, &createInfo, nullptr, &shaderModules[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed To Create Shader Module");
        
        
        std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages(1);
        pipelineShaderStages[0] = {};
        pipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStages[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineShaderStages[0].module = shaderModules[0];
        pipelineShaderStages[0].pName = "main";
        

        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(4);
        descriptorSetLayoutBindings[0].binding = 0;
        descriptorSetLayoutBindings[0].descriptorCount = 1;
        descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        descriptorSetLayoutBindings[1].binding = 1;
        descriptorSetLayoutBindings[1].descriptorCount = 1;
        descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        descriptorSetLayoutBindings[2].binding = 2;
        descriptorSetLayoutBindings[2].descriptorCount = 1;
        descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        descriptorSetLayoutBindings[3].binding = 3;
        descriptorSetLayoutBindings[3].descriptorCount = 1;
        descriptorSetLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[3].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        descriptorSetLayoutCreateInfo.bindingCount = 4,
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        VkDescriptorSetLayout descriptorSetLayout;
        if(vkCreateDescriptorSetLayout(engine.logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE DESCRIPTOR SET LAYOUT");


        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(engine.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &lightCullingPrepass.pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE PIPELINE LAYOUT\n");


        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.stage = pipelineShaderStages[0];
        computePipelineCreateInfo.layout = lightCullingPrepass.pipelineLayout;
        if(vkCreateComputePipelines(engine.logicalDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &lightCullingPrepass.computePipeline) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE COMPUTE PIPELINES");


        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = engine.physicalDevice.queueIndex;
        VkCommandPool commandPool;
        if(vkCreateCommandPool(engine.logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE COMMAND POOL\n");
        
        
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
        lightCullingPrepass.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        if(vkAllocateCommandBuffers(engine.logicalDevice, &commandBufferAllocateInfo, lightCullingPrepass.commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE COMMAND BUFFERS\n");
        
        
        lightCullingPrepass.fences.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.pNext = nullptr;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            if(vkCreateFence(engine.logicalDevice, &fenceCreateInfo, nullptr, &lightCullingPrepass.fences[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FENCES\n");
        }

            
        lightCullingPrepass.finishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT) ;
        for( int i = 0 ; i < MAX_FRAMES_IN_FLIGHT ; i++ ){
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if(vkCreateSemaphore(engine.logicalDevice, &info, nullptr, &lightCullingPrepass.finishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE SEMAPHORES");
        }


        lightCullingPrepass.uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            lightCullingPrepass.uniformBuffers[i] = Buffer::BuildHostVisibleBuffer(engine.logicalDevice, engine.physicalDevice.handle, sizeof(LightCullingPrepassUniformVariables), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        
        std::vector<VkDescriptorBufferInfo> uniformDescriptorBuffers(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            uniformDescriptorBuffers[i] = VkDescriptorBufferInfo{
                .buffer = lightCullingPrepass.uniformBuffers[i].bufferPack.buffer,
                .offset = 0,
                .range = sizeof(LightCullingPrepassUniformVariables)
            };
        
        std::vector<BufferInfo> tileLightCullingPrepassBuffers(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            tileLightCullingPrepassBuffers[i] = Buffer::BuildBuffer(engine.logicalDevice, engine.physicalDevice.handle, lightCullingPrepass.totalNumTiles * sizeof(glm::vec2), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        lightCullingPrepass.tileLightCullingPrepassDescriptorBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            lightCullingPrepass.tileLightCullingPrepassDescriptorBuffers[i] = VkDescriptorBufferInfo{
                .buffer = tileLightCullingPrepassBuffers[i].buffer,
                .offset = 0,
                .range = lightCullingPrepass.totalNumTiles * sizeof(glm::vec2)
            };
        
        std::vector<BufferInfo> tilesGeometryBitMaskBuffers(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            tilesGeometryBitMaskBuffers[i] = Buffer::BuildBuffer(engine.logicalDevice, engine.physicalDevice.handle, lightCullingPrepass.totalNumTiles * sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        lightCullingPrepass.tilesGeometryBitMaskDescriptorBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            lightCullingPrepass.tilesGeometryBitMaskDescriptorBuffers[i] = VkDescriptorBufferInfo{
                .buffer = tilesGeometryBitMaskBuffers[i].buffer,
                .offset = 0,
                .range = lightCullingPrepass.totalNumTiles * sizeof(unsigned int)
            };
        
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes(3);
        descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorPoolSizes[0].descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT;
        descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;
        descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorPoolSizes[2].descriptorCount = MAX_FRAMES_IN_FLIGHT;


        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{} ;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO ;
        descriptorPoolCreateInfo.poolSizeCount = 3;
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
        VkDescriptorPool descriptorPool;
        if(vkCreateDescriptorPool(engine.logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE DESCRIPTOR POOL");


        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
        descriptorSetAllocateInfo.pNext = nullptr;
        lightCullingPrepass.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        vkAllocateDescriptorSets(engine.logicalDevice, &descriptorSetAllocateInfo, lightCullingPrepass.descriptorSets.data());
        
        
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            std::vector<VkWriteDescriptorSet> writeDescriptorSets(4);
            writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[0].dstSet = lightCullingPrepass.descriptorSets[i];
            writeDescriptorSets[0].dstBinding = 0;
            writeDescriptorSets[0].dstArrayElement = 0;
            writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSets[0].descriptorCount = 1;            
            writeDescriptorSets[0].pImageInfo = &depthPrepass.descriptorImages[i];
            writeDescriptorSets[0].pBufferInfo = nullptr;
            
            writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[1].dstSet = lightCullingPrepass.descriptorSets[i];
            writeDescriptorSets[1].dstBinding = 1;
            writeDescriptorSets[1].dstArrayElement = 0;
            writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[1].descriptorCount = 1;
            writeDescriptorSets[1].pImageInfo = nullptr;
            
            writeDescriptorSets[1].pBufferInfo = &uniformDescriptorBuffers[i];
            writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[2].dstSet = lightCullingPrepass.descriptorSets[i];
            writeDescriptorSets[2].dstBinding = 2;
            writeDescriptorSets[2].dstArrayElement = 0;
            writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[2].descriptorCount = 1;
            writeDescriptorSets[2].pImageInfo = nullptr;
            writeDescriptorSets[2].pBufferInfo = &lightCullingPrepass.tileLightCullingPrepassDescriptorBuffers[i];
            
            writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[3].dstSet = lightCullingPrepass.descriptorSets[i];
            writeDescriptorSets[3].dstBinding = 3;
            writeDescriptorSets[3].dstArrayElement = 0;
            writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[3].descriptorCount = 1;
            writeDescriptorSets[3].pImageInfo = nullptr;
            writeDescriptorSets[3].pBufferInfo = &lightCullingPrepass.tilesGeometryBitMaskDescriptorBuffers[i];
            vkUpdateDescriptorSets(engine.logicalDevice, 4, writeDescriptorSets.data(), 0, NULL);
        };
    }

    static void Record(EngineInfo& engine, CameraInfo& camera, LightCullingPrepassInfo& lightCullingPrepass, uint32_t frame)
    {
        vkWaitForFences(engine.logicalDevice, 1, &lightCullingPrepass.fences[frame], VK_TRUE, UINT64_MAX);
        
        vkResetFences(engine.logicalDevice, 1, &lightCullingPrepass.fences[frame]) ;
        
        if(vkResetCommandBuffer(lightCullingPrepass.commandBuffers[frame], 0) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO RESET COMMAND BUFFER\n");
        
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(lightCullingPrepass.commandBuffers[frame], &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BEGIN RECORDING COMMAND BUFFER\n");
        
        lightCullingPrepass.uniformData.cameraNearPlane = camera.config.nearClippingPlane;
        lightCullingPrepass.uniformData.cameraFarPlane = camera.config.farClippingPlane;
        lightCullingPrepass.uniformData.numCellsPerTile = lightCullingPrepass.numCellsPerTile;

        memcpy(lightCullingPrepass.uniformBuffers[frame].memoryPointer, &lightCullingPrepass.uniformData, sizeof(LightCullingPrepassUniformVariables));
        vkCmdBindPipeline(lightCullingPrepass.commandBuffers[frame], VK_PIPELINE_BIND_POINT_COMPUTE, lightCullingPrepass.computePipeline);
        vkCmdBindDescriptorSets(lightCullingPrepass.commandBuffers[frame], VK_PIPELINE_BIND_POINT_COMPUTE, lightCullingPrepass.pipelineLayout, 0, 1, &lightCullingPrepass.descriptorSets[frame], 0, nullptr);
        vkCmdDispatch(lightCullingPrepass.commandBuffers[frame], lightCullingPrepass.numTiles2D.x, lightCullingPrepass.numTiles2D.y, 1);
        vkEndCommandBuffer(lightCullingPrepass.commandBuffers[frame]);
    }

    static void Submit(EngineInfo& engine, DepthPrepassInfo& depthPrepass, LightCullingPrepassInfo& lightCullingPrepass, uint32_t frame)
    {
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &lightCullingPrepass.commandBuffers[frame];
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &depthPrepass.finishedSemaphores[frame];
        VkPipelineStageFlags stageFlag = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        info.pWaitDstStageMask = &stageFlag;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &lightCullingPrepass.finishedSemaphores[frame];
        
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, lightCullingPrepass.fences[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
    }
};
