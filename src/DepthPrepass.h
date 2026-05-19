#pragma once
#include "Model.h".
#include "Engine.h"
#include "Buffer.h"




class DepthPrepass{
    
public:
    
    static void Build(EngineInfo& engine, DepthPrepassInfo& depthPrepass)
    {
        std::vector<VkShaderModule> shaderModules(1);

        std::vector<uint32_t> shaderData;
        std::ifstream file("../Resources/Shaders/DepthPrepass/DepthPrepass_vs.spv", std::ios::binary | std::ios::ate ) ;
        if(!file.is_open())
            throw std::runtime_error("FAILURE TO OPEN SHADER FILE FOR READING");
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        shaderData.resize(fileSize / sizeof(uint32_t));
        file.read( reinterpret_cast<char*>(shaderData.data()), fileSize);
        
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderData.size() * sizeof(uint32_t);
        createInfo.pCode = shaderData.data();
        if(vkCreateShaderModule(engine.logicalDevice, &createInfo, nullptr, &shaderModules[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed To Create Shader Module");
        
        std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages(1);
        pipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipelineShaderStages[0].module = shaderModules[0];
        pipelineShaderStages[0].pName = "main";


        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes(1);
        vertexInputAttributes[0].location = 0;
        vertexInputAttributes[0].binding = 0;
        vertexInputAttributes[0].offset = 0;
        vertexInputAttributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        
        std::vector<VkVertexInputBindingDescription> vertexInputBindings(1);
        vertexInputBindings[0].binding = 0;
        vertexInputBindings[0].stride = 3 * sizeof(float);
        vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;


        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = engine.depthFormat;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.pColorAttachments = nullptr;
        subpassDescription.colorAttachmentCount = 0;
        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 0;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;


        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO ;
        info.attachmentCount = 1;
        info.pAttachments = &attachmentDescription;
        info.pSubpasses = &subpassDescription;
        info.subpassCount = 1;
        info.pDependencies = &subpassDependency;
        info.dependencyCount = 1;
        if(vkCreateRenderPass(engine.logicalDevice, &info, nullptr, &depthPrepass.renderPass) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE RENDER PASS");


        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
        descriptorSetLayoutBinding.binding = 0;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        descriptorSetLayoutCreateInfo.bindingCount = 1,
        descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
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
        if(vkCreatePipelineLayout(engine.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &depthPrepass.pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE PIPELINE LAYOUT\n");


        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreate;
        dynamicStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreate.dynamicStateCount = 2;
        dynamicStateCreate.pDynamicStates = dynamicStates.data();
        
        
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreate{};
        depthStencilStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreate.depthTestEnable = VK_TRUE;
        depthStencilStateCreate.depthWriteEnable = VK_TRUE;
        depthStencilStateCreate.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilStateCreate.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreate.stencilTestEnable = VK_FALSE;
        
        
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreate{};
        colorBlendStateCreate.attachmentCount = 0;
        colorBlendStateCreate.pAttachments = nullptr;
        colorBlendStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreate.logicOpEnable = VK_FALSE;
        colorBlendStateCreate.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreate.blendConstants[0] = 0.0f;
        colorBlendStateCreate.blendConstants[1] = 0.0f;
        colorBlendStateCreate.blendConstants[2] = 0.0f;
        colorBlendStateCreate.blendConstants[3] = 0.0f;
        
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreate{};
        inputAssemblyStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreate.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreate.primitiveRestartEnable = VK_FALSE;
        
        
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreate{};
        rasterizationStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreate.depthClampEnable = VK_FALSE;
        rasterizationStateCreate.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreate.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreate.lineWidth = 1.0f;
        rasterizationStateCreate.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreate.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCreate.depthBiasEnable = VK_FALSE;
        rasterizationStateCreate.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreate.depthBiasClamp = 0.0f;
        rasterizationStateCreate.depthBiasSlopeFactor = 0.0f;
        
        
        VkPipelineMultisampleStateCreateInfo multisampleStateCreate{};
        multisampleStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreate.sampleShadingEnable = VK_FALSE;
        multisampleStateCreate.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreate.minSampleShading = 1.0f;
        multisampleStateCreate.pSampleMask = nullptr;
        multisampleStateCreate.alphaToCoverageEnable = VK_FALSE;
        multisampleStateCreate.alphaToOneEnable = VK_FALSE;
        
        
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreate{};
        vertexInputStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO ;
        vertexInputStateCreate.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
        vertexInputStateCreate.pVertexBindingDescriptions = vertexInputBindings.data();
        vertexInputStateCreate.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputStateCreate.pVertexAttributeDescriptions = vertexInputAttributes.data();

        VkPipelineViewportStateCreateInfo viewportStateCreate{};
        viewportStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO ;
        viewportStateCreate.viewportCount = 1;
        viewportStateCreate.scissorCount = 1;


        VkGraphicsPipelineCreateInfo graphicsPipelineCreate{};
        graphicsPipelineCreate.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreate.stageCount = static_cast<uint32_t>(pipelineShaderStages.size());
        graphicsPipelineCreate.pStages = pipelineShaderStages.data();
        graphicsPipelineCreate.pVertexInputState = &vertexInputStateCreate;
        graphicsPipelineCreate.pInputAssemblyState = &inputAssemblyStateCreate;
        graphicsPipelineCreate.pViewportState = &viewportStateCreate;
        graphicsPipelineCreate.pRasterizationState = &rasterizationStateCreate;
        graphicsPipelineCreate.pMultisampleState = &multisampleStateCreate;
        graphicsPipelineCreate.pDepthStencilState = &depthStencilStateCreate;
        graphicsPipelineCreate.pColorBlendState = &colorBlendStateCreate;
        graphicsPipelineCreate.pDynamicState = &dynamicStateCreate;
        graphicsPipelineCreate.layout = depthPrepass.pipelineLayout;
        graphicsPipelineCreate.renderPass = depthPrepass.renderPass;
        graphicsPipelineCreate.subpass = 0;
        graphicsPipelineCreate.basePipelineHandle = VK_NULL_HANDLE;
        if(vkCreateGraphicsPipelines(engine.logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreate, nullptr, &depthPrepass.graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE GRAPHICS PIPELINE");


        std::vector<ImageInfo> imagePacks(engine.swapChain.imageViews.size());
        for(int i = 0; i < engine.swapChain.imageViews.size(); i++){
            VkImageCreateInfo imageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .extent.width = engine.swapChain.extent.width,
                .extent.height = engine.swapChain.extent.height,
                .extent.depth = 1,
                .mipLevels = 1,
                .arrayLayers = 1,
                .format = engine.depthFormat,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .flags = 0
            };
            if(vkCreateImage(engine.logicalDevice, &imageCreateInfo, nullptr, &imagePacks[i].image) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE IMAGE");
            
            VkMemoryRequirements memRequirements ;
            vkGetImageMemoryRequirements(engine.logicalDevice, imagePacks[i].image, &memRequirements);
            VkMemoryAllocateInfo allocInfo{} ;
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size ;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(engine.physicalDevice.handle, &memoryProperties);
            for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
                if((memRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
                    allocInfo.memoryTypeIndex = i;
                    break;
                }
            }
            if(vkAllocateMemory(engine.logicalDevice, &allocInfo, nullptr, &imagePacks[i].imageMemory) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO ALLOCATE IMAGE MEMORY ");
            
            vkBindImageMemory(engine.logicalDevice, imagePacks[i].image, imagePacks[i].imageMemory, 0) ;
        }


        depthPrepass.imageViews.resize(engine.swapChain.imageViews.size()) ;
        for(uint32_t i = 0; i < engine.swapChain.imageViews.size(); i++){
            VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = imagePacks[i].image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = engine.depthFormat,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
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
            if(vkCreateImageView(engine.logicalDevice, &viewInfo, nullptr, &depthPrepass.imageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("FAILURE TO CREATE IMAGE VIEW");
        }


        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(engine.physicalDevice.handle, &properties);
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.anisotropyEnable = VK_FALSE;
        samplerCreateInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;
        if(vkCreateSampler(engine.logicalDevice, &samplerCreateInfo, nullptr, &depthPrepass.sampler) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE SAMPLER\n");


        depthPrepass.descriptorImages.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            depthPrepass.descriptorImages[i] = VkDescriptorImageInfo{
                .sampler = depthPrepass.sampler,
                .imageView = depthPrepass.imageViews[i],
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };


        depthPrepass.frameBuffers.resize(engine.swapChain.imageViews.size());
        for(size_t i = 0; i < engine.swapChain.imageViews.size(); i++){
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = depthPrepass.renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = &depthPrepass.imageViews[i];
            framebufferCreateInfo.width = engine.swapChain.extent.width;
            framebufferCreateInfo.height = engine.swapChain.extent.height;
            framebufferCreateInfo.layers = 1;
            if(vkCreateFramebuffer(engine.logicalDevice, &framebufferCreateInfo, nullptr, &depthPrepass.frameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FRAMEBUFFERS");
        }
        
        
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
        depthPrepass.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        if(vkAllocateCommandBuffers(engine.logicalDevice, &commandBufferAllocateInfo, depthPrepass.commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE COMMAND BUFFERS\n");


        depthPrepass.fences.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.pNext = nullptr;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            if(vkCreateFence(engine.logicalDevice, &fenceCreateInfo, nullptr, &depthPrepass.fences[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FENCES\n");
        }


        depthPrepass.finishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if(vkCreateSemaphore(engine.logicalDevice, &info, nullptr, &depthPrepass.finishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE SEMAPHORES");
        }


        VkDescriptorPoolSize descriptorPoolSize{};
        descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSize.descriptorCount = MAX_FRAMES_IN_FLIGHT;
        
        
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{} ;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;
        descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
        VkDescriptorPool descriptorPool;
        if(vkCreateDescriptorPool(engine.logicalDevice, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE DESCRIPTOR POOL");
        
        depthPrepass.vertexShaderUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            depthPrepass.vertexShaderUniformBuffers[i] = Buffer::BuildHostVisibleBuffer(engine.logicalDevice, engine.physicalDevice.handle, sizeof(DepthPrepassShaderUniform::Vertex), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        depthPrepass.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
        descriptorSetAllocateInfo.pNext = nullptr;
        vkAllocateDescriptorSets(engine.logicalDevice, &descriptorSetAllocateInfo, depthPrepass.descriptorSets.data());
        
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = depthPrepass.descriptorSets[i];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.descriptorCount = 1;
            VkDescriptorBufferInfo descriptorBufferInfo{
                .buffer = depthPrepass.vertexShaderUniformBuffers[i].bufferPack.buffer,
                .offset = 0,
                .range = sizeof(DepthPrepassShaderUniform::Vertex)
            };
            writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
            writeDescriptorSet.pImageInfo = nullptr;
            vkUpdateDescriptorSets(engine.logicalDevice, 1, &writeDescriptorSet, 0, NULL);
        }
    }

    static void Record(EngineInfo& engine, DepthPrepassInfo& depthPrepass, ModelInfo& model, CameraInfo& camera, uint32_t frame)
    {
        if(vkResetCommandBuffer(depthPrepass.commandBuffers[frame], 0) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO RESET COMMAND BUFFER\n");
        
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(depthPrepass.commandBuffers[frame], &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BEGIN RECORDING COMMAND BUFFER\n");
        
        VkClearValue clearValue{
            .depthStencil = {1.0f, 0}
        };
        VkRenderPassBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = depthPrepass.renderPass,
            .framebuffer = depthPrepass.frameBuffers[frame],
            .renderArea.offset = {0, 0},
            .renderArea.extent = engine.swapChain.extent,
            .clearValueCount = 1,
            .pClearValues = &clearValue
        };
        vkCmdBeginRenderPass(depthPrepass.commandBuffers[frame], &info, VK_SUBPASS_CONTENTS_INLINE);
        
        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(engine.swapChain.extent.width),
            .height = static_cast<float>(engine.swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(depthPrepass.commandBuffers[frame], 0, 1, &viewport);

        VkRect2D scissor{
            .offset = { 0, 0 },
            .extent = engine.swapChain.extent
        };
        vkCmdSetScissor(depthPrepass.commandBuffers[frame], 0, 1, &scissor);
        
        vkCmdBindPipeline(depthPrepass.commandBuffers[frame], VK_PIPELINE_BIND_POINT_GRAPHICS, depthPrepass.graphicsPipeline);
        memcpy(depthPrepass.vertexShaderUniformBuffers[frame].memoryPointer, &depthPrepass.vsData, sizeof(DepthPrepassShaderUniform::Vertex));
        vkCmdBindDescriptorSets(depthPrepass.commandBuffers[frame], VK_PIPELINE_BIND_POINT_GRAPHICS, depthPrepass.pipelineLayout, 0, 1, &depthPrepass.descriptorSets[frame], 0, nullptr);
        vkCmdBindVertexBuffers(depthPrepass.commandBuffers[frame], 0, static_cast<uint32_t>(depthPrepass.vertexBuffers.size()), depthPrepass.vertexBuffers.data(), depthPrepass.vertexBufferOffsets.data());
        vkCmdBindIndexBuffer(depthPrepass.commandBuffers[frame], model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(depthPrepass.commandBuffers[frame], model.drawCommandsBuffer.buffer, 0, model.numDrawCommands, sizeof(VkDrawIndexedIndirectCommand));
        vkCmdEndRenderPass(depthPrepass.commandBuffers[frame]);
        vkEndCommandBuffer(depthPrepass.commandBuffers[frame]);
    }

    static void Submit(DepthPrepassInfo& depthPrepass, EngineInfo& engine, uint32_t frame)
    {
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &depthPrepass.commandBuffers[frame];
        info.waitSemaphoreCount = 0;
        info.pWaitSemaphores = nullptr;
        info.pWaitDstStageMask = nullptr;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &depthPrepass.finishedSemaphores[frame];
        
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, depthPrepass.fences[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
    }
};
