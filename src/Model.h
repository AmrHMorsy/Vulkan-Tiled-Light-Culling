#pragma once
#include "DepthPrepass.h"
#include "Engine.h"
#include "Loader.h"
#include "Buffer.h"
#include "Texture.h"


class Model{
    
public:
    
    static void Load(EngineInfo& engine, ModelInfo& model, DepthPrepassInfo& depthPrepass, const std::vector<ModelConfig>& modelConfigs)
    {
        Loader::Load(engine, model, depthPrepass, modelConfigs);
        
        model.albedoTextures.resize(MAX_NUM_MODELS);
        for(int i = 0; i < modelConfigs.size(); i++)
            Texture::Build(engine, model.albedoTextures[i], modelConfigs[i].albedoFilePath);
    }
    
    static void Build(EngineInfo& engine, ModelInfo& model, VkRenderPass& renderPass, LightCullingInfo& lightCulling, LightCullingPrepassInfo& lightCullingPrepass, std::vector<LightInfo> lights, DepthPrepassInfo& depthPrepass, bool lightCullingEnabled, bool heatMap, bool displayAlbedo, bool displayNormal)
    {
        model.fsData.numLights = lights.size();
        model.fsData.lightCullingEnabled = lightCullingEnabled;
        model.fsData.heatMap_numCellsPerTile_albedo_normal.r = heatMap;
        model.fsData.heatMap_numCellsPerTile_albedo_normal.g = lightCullingPrepass.numCellsPerTile;
        model.fsData.heatMap_numCellsPerTile_albedo_normal.b = displayAlbedo;
        model.fsData.heatMap_numCellsPerTile_albedo_normal.a = displayNormal;
        model.fsData.numTiles2D = lightCulling.numTiles2D;
        model.fsData.screenResolution = glm::vec2(engine.swapChain.extent.width, engine.swapChain.extent.height);
        for(size_t i = 0; i < lights.size(); i++){
            model.fsData.lightPositions[i] = lights[i].config.position;
            model.fsData.lightColors[i] = lights[i].config.intensity * lights[i].config.color;
        }

        std::vector<uint32_t> vsShaderData;
        std::ifstream vsFile("../Resources/shaders/Main/vert.spv", std::ios::binary | std::ios::ate ) ;
        if(!vsFile.is_open())
            throw std::runtime_error("FAILURE TO OPEN SHADER FILE FOR READING");
        size_t fileSize = vsFile.tellg();
        vsFile.seekg(0, std::ios::beg);
        vsShaderData.resize(fileSize / sizeof(uint32_t));
        vsFile.read(reinterpret_cast<char*>(vsShaderData.data()), fileSize);

        std::vector<uint32_t> fsShaderData;
        std::ifstream fsFile("../Resources/shaders/Main/frag.spv", std::ios::binary | std::ios::ate);
        if(!fsFile.is_open())
            throw std::runtime_error("FAILURE TO OPEN SHADER FILE FOR READING");
        fileSize = fsFile.tellg();
        fsFile.seekg(0, std::ios::beg);
        fsShaderData.resize(fileSize / sizeof(uint32_t));
        fsFile.read(reinterpret_cast<char*>(fsShaderData.data()), fileSize);

        std::vector<VkShaderModule> shaderModules(2);
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = vsShaderData.size() * sizeof(uint32_t);
        createInfo.pCode = vsShaderData.data();
        if(vkCreateShaderModule(engine.logicalDevice, &createInfo, nullptr, &shaderModules[0]) != VK_SUCCESS)
            throw std::runtime_error("Failed To Create Shader Module");
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = fsShaderData.size() * sizeof(uint32_t);
        createInfo.pCode = fsShaderData.data();
        if(vkCreateShaderModule(engine.logicalDevice, &createInfo, nullptr, &shaderModules[1]) != VK_SUCCESS)
            throw std::runtime_error("Failed To Create Shader Module");

        std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages(2);
        pipelineShaderStages[0] = {};
        pipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipelineShaderStages[0].module = shaderModules[0];
        pipelineShaderStages[0].pName = "main";
        pipelineShaderStages[1] = {};
        pipelineShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipelineShaderStages[1].module = shaderModules[1];
        pipelineShaderStages[1].pName = "main";

        std::vector<VkVertexInputAttributeDescription> vertexInputAttributes(3);
        vertexInputAttributes[0].location = 0;
        vertexInputAttributes[0].binding = 0;
        vertexInputAttributes[0].offset = 0;
        vertexInputAttributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertexInputAttributes[1].location = 1;
        vertexInputAttributes[1].binding = 1;
        vertexInputAttributes[1].offset = 0;
        vertexInputAttributes[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vertexInputAttributes[2].location = 2;
        vertexInputAttributes[2].binding = 2;
        vertexInputAttributes[2].offset = 0;
        vertexInputAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;

        std::vector<VkVertexInputBindingDescription> vertexInputBindings(3);
        vertexInputBindings[0].binding = 0;
        vertexInputBindings[0].stride = 3 * sizeof(float);
        vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputBindings[1].binding = 1;
        vertexInputBindings[1].stride = 3 * sizeof(float);
        vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputBindings[2].binding = 2;
        vertexInputBindings[2].stride = 2 * sizeof(float);
        vertexInputBindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(7);
        descriptorSetLayoutBindings[0].binding = 0;
        descriptorSetLayoutBindings[0].descriptorCount = 1;
        descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        descriptorSetLayoutBindings[1].binding = 1;
        descriptorSetLayoutBindings[1].descriptorCount = 1;
        descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings[2].binding = 2;
        descriptorSetLayoutBindings[2].descriptorCount = 1;
        descriptorSetLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[2].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings[3].binding = 3;
        descriptorSetLayoutBindings[3].descriptorCount = 1;
        descriptorSetLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[3].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings[4].binding = 4;
        descriptorSetLayoutBindings[4].descriptorCount = 1;
        descriptorSetLayoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[4].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings[5].binding = 5;
        descriptorSetLayoutBindings[5].descriptorCount = 1;
        descriptorSetLayoutBindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorSetLayoutBindings[5].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        descriptorSetLayoutBindings[6].binding = 6;
        descriptorSetLayoutBindings[6].descriptorCount = MAX_NUM_MODELS;
        descriptorSetLayoutBindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorSetLayoutBindings[6].pImmutableSamplers = nullptr;
        descriptorSetLayoutBindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        descriptorSetLayoutCreateInfo.bindingCount = 7,
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        if(vkCreateDescriptorSetLayout(engine.logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE DESCRIPTOR SET LAYOUT");
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
        if(vkCreatePipelineLayout(engine.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &model.pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE PIPELINE LAYOUT\n");

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicStateCreate{};
        dynamicStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreate.dynamicStateCount = 2;
        dynamicStateCreate.pDynamicStates = dynamicStates.data();

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreate{};
        depthStencilStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreate.depthTestEnable = VK_TRUE;
        depthStencilStateCreate.depthWriteEnable = VK_FALSE;
        depthStencilStateCreate.depthCompareOp = VK_COMPARE_OP_EQUAL;
        depthStencilStateCreate.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreate.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreate{};
        colorBlendStateCreate.attachmentCount = 1;
        colorBlendStateCreate.pAttachments = &colorBlendAttachment;
        colorBlendStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreate.logicOpEnable = VK_FALSE;
        colorBlendStateCreate.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreate.blendConstants[0] = 0.0f;
        colorBlendStateCreate.blendConstants[1] = 0.0f;
        colorBlendStateCreate.blendConstants[2] = 0.0f;
        colorBlendStateCreate.blendConstants[3] = 0.0f;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreate{};
        inputAssemblyStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO ;
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
        graphicsPipelineCreate.layout = model.pipelineLayout;
        graphicsPipelineCreate.renderPass = renderPass;
        graphicsPipelineCreate.subpass = 0;
        graphicsPipelineCreate.basePipelineHandle = VK_NULL_HANDLE;
        if(vkCreateGraphicsPipelines(engine.logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreate, nullptr, &model.graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE GRAPHICS PIPELINE");

        model.vertexShaderUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            model.vertexShaderUniformBuffers[i] = Buffer::BuildHostVisibleBuffer(engine.logicalDevice, engine.physicalDevice.handle, sizeof(ModelShaderUniform::Vertex), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        model.vertexShaderUniformDescriptorBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            model.vertexShaderUniformDescriptorBuffers[i] = VkDescriptorBufferInfo{
                .buffer = model.vertexShaderUniformBuffers[i].bufferPack.buffer,
                .offset = 0,
                .range = sizeof(ModelShaderUniform::Vertex)
            };
        
        model.fragmentShaderUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            model.fragmentShaderUniformBuffers[i] = Buffer::BuildHostVisibleBuffer(engine.logicalDevice, engine.physicalDevice.handle, sizeof(ModelShaderUniform::Fragment), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
        model.fragmentShaderUniformDescriptorBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            model.fragmentShaderUniformDescriptorBuffers[i] = VkDescriptorBufferInfo{
                .buffer = model.fragmentShaderUniformBuffers[i].bufferPack.buffer,
                .offset = 0,
                .range = sizeof(ModelShaderUniform::Fragment)
            };
        
        model.albedoDescriptorImages.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            model.albedoDescriptorImages[i].resize(MAX_NUM_MODELS);
            for(int j = 0; j < MAX_NUM_MODELS; j++)
                model.albedoDescriptorImages[i][j] = VkDescriptorImageInfo{
                    .sampler = VK_NULL_HANDLE,
                    .imageView = VK_NULL_HANDLE,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
            for(int j = 0; j < model.albedoTextures.size(); j++)
                model.albedoDescriptorImages[i][j] = VkDescriptorImageInfo{
                    .sampler = model.albedoTextures[j].sampler,
                    .imageView = model.albedoTextures[j].imageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
        }
        
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes(3);
        descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorPoolSizes[0].descriptorCount = 4 * MAX_FRAMES_IN_FLIGHT;
        descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorPoolSizes[1].descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT;
        descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorPoolSizes[2].descriptorCount = MAX_NUM_MODELS * MAX_FRAMES_IN_FLIGHT;
        
        
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{} ;
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data() ;
        descriptorPoolCreateInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
        model.descriptorPool;
        if(vkCreateDescriptorPool(engine.logicalDevice, &descriptorPoolCreateInfo, nullptr, &model.descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE DESCRIPTOR POOL");


        model.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = model.descriptorPool;
        descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();
        descriptorSetAllocateInfo.pNext = nullptr;
        if(vkAllocateDescriptorSets(engine.logicalDevice, &descriptorSetAllocateInfo, model.descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("FAILURE TO ALLOCATE DESCRIPTOR SETS");
        
        
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            std::vector<VkWriteDescriptorSet> writeDescriptorSets(7);
            writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[0].dstSet = model.descriptorSets[i];
            writeDescriptorSets[0].dstBinding = 0;
            writeDescriptorSets[0].dstArrayElement = 0;
            writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[0].descriptorCount = 1;
            writeDescriptorSets[0].pBufferInfo = &model.vertexShaderUniformDescriptorBuffers[i];
            writeDescriptorSets[0].pImageInfo = nullptr;
            writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[1].dstSet = model.descriptorSets[i];
            writeDescriptorSets[1].dstBinding = 1;
            writeDescriptorSets[1].dstArrayElement = 0;
            writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[1].descriptorCount = 1;
            writeDescriptorSets[1].pBufferInfo = &model.fragmentShaderUniformDescriptorBuffers[i];
            writeDescriptorSets[1].pImageInfo = nullptr;
            writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[2].dstSet = model.descriptorSets[i];
            writeDescriptorSets[2].dstBinding = 2;
            writeDescriptorSets[2].dstArrayElement = 0;
            writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[2].descriptorCount = 1;
            writeDescriptorSets[2].pBufferInfo = &lightCullingPrepass.tileLightCullingPrepassDescriptorBuffers[i];
            writeDescriptorSets[2].pImageInfo = nullptr;
            writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[3].dstSet = model.descriptorSets[i];
            writeDescriptorSets[3].dstBinding = 3;
            writeDescriptorSets[3].dstArrayElement = 0;
            writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[3].descriptorCount = 1;
            writeDescriptorSets[3].pBufferInfo = &lightCulling.tileLightCountDescriptorBuffers[i];
            writeDescriptorSets[3].pImageInfo = nullptr;
            writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[4].dstSet = model.descriptorSets[i];
            writeDescriptorSets[4].dstBinding = 4;
            writeDescriptorSets[4].dstArrayElement = 0;
            writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[4].descriptorCount = 1;
            writeDescriptorSets[4].pBufferInfo = &lightCulling.tileLightIndicesDescriptorBuffers[i];
            writeDescriptorSets[4].pImageInfo = nullptr;
            writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[5].dstSet = model.descriptorSets[i];
            writeDescriptorSets[5].dstBinding = 5;
            writeDescriptorSets[5].dstArrayElement = 0;
            writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[5].descriptorCount = 1;
            writeDescriptorSets[5].pBufferInfo = &lightCulling.maxLightCountDescriptorBuffers[i];
            writeDescriptorSets[5].pImageInfo = nullptr;
            writeDescriptorSets[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[6].dstSet = model.descriptorSets[i];
            writeDescriptorSets[6].dstBinding = 6;
            writeDescriptorSets[6].dstArrayElement = 0;
            writeDescriptorSets[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSets[6].descriptorCount = MAX_NUM_MODELS;
            writeDescriptorSets[6].pBufferInfo = nullptr;
            writeDescriptorSets[6].pImageInfo = model.albedoDescriptorImages[i].data();
            vkUpdateDescriptorSets(engine.logicalDevice, 7, writeDescriptorSets.data(), 0, NULL);
        }
    }
    
    static void Render(VkDevice logicalDevice, std::vector<ModelInfo>& models, VkCommandBuffer& commandBuffer, uint32_t frame)
    {
        for(size_t i = 0; i < models.size(); i++)
            Render(logicalDevice, models[i], commandBuffer, frame);
    }
    
    static void Render(VkDevice logicalDevice, ModelInfo& model, VkCommandBuffer& commandBuffer, uint32_t frame)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model.graphicsPipeline);
        memcpy(model.vertexShaderUniformBuffers[frame].memoryPointer, &model.vsData, sizeof(ModelShaderUniform::Vertex));
        memcpy(model.fragmentShaderUniformBuffers[frame].memoryPointer, &model.fsData, sizeof(ModelShaderUniform::Fragment));
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, model.pipelineLayout, 0, 1, &model.descriptorSets[frame], 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, static_cast<uint32_t>(model.vertexBuffers.size()), model.vertexBuffers.data(), model.vertexBufferOffsets.data());
        vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexedIndirect(commandBuffer, model.drawCommandsBuffer.buffer, 0, model.numDrawCommands, sizeof(VkDrawIndexedIndirectCommand));
    }
    
    static void UpdateNumCellsPerTile(EngineInfo& engine, LightCullingPrepassInfo& lightCullingPrepass, LightCullingInfo& lightCulling, ModelInfo& model)
    {        
        for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            std::vector<VkWriteDescriptorSet> writeDescriptorSets(7);
            writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[0].dstSet = model.descriptorSets[i];
            writeDescriptorSets[0].dstBinding = 0;
            writeDescriptorSets[0].dstArrayElement = 0;
            writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[0].descriptorCount = 1;
            writeDescriptorSets[0].pBufferInfo = &model.vertexShaderUniformDescriptorBuffers[i];
            writeDescriptorSets[0].pImageInfo = nullptr;
            writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[1].dstSet = model.descriptorSets[i];
            writeDescriptorSets[1].dstBinding = 1;
            writeDescriptorSets[1].dstArrayElement = 0;
            writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSets[1].descriptorCount = 1;
            writeDescriptorSets[1].pBufferInfo = &model.fragmentShaderUniformDescriptorBuffers[i];
            writeDescriptorSets[1].pImageInfo = nullptr;
            writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[2].dstSet = model.descriptorSets[i];
            writeDescriptorSets[2].dstBinding = 2;
            writeDescriptorSets[2].dstArrayElement = 0;
            writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[2].descriptorCount = 1;
            writeDescriptorSets[2].pBufferInfo = &lightCullingPrepass.tileLightCullingPrepassDescriptorBuffers[i];
            writeDescriptorSets[2].pImageInfo = nullptr;
            writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[3].dstSet = model.descriptorSets[i];
            writeDescriptorSets[3].dstBinding = 3;
            writeDescriptorSets[3].dstArrayElement = 0;
            writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[3].descriptorCount = 1;
            writeDescriptorSets[3].pBufferInfo = &lightCulling.tileLightCountDescriptorBuffers[i];
            writeDescriptorSets[3].pImageInfo = nullptr;
            writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[4].dstSet = model.descriptorSets[i];
            writeDescriptorSets[4].dstBinding = 4;
            writeDescriptorSets[4].dstArrayElement = 0;
            writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[4].descriptorCount = 1;
            writeDescriptorSets[4].pBufferInfo = &lightCulling.tileLightIndicesDescriptorBuffers[i];
            writeDescriptorSets[4].pImageInfo = nullptr;
            writeDescriptorSets[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[5].dstSet = model.descriptorSets[i];
            writeDescriptorSets[5].dstBinding = 5;
            writeDescriptorSets[5].dstArrayElement = 0;
            writeDescriptorSets[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSets[5].descriptorCount = 1;
            writeDescriptorSets[5].pBufferInfo = &lightCulling.maxLightCountDescriptorBuffers[i];
            writeDescriptorSets[5].pImageInfo = nullptr;
            writeDescriptorSets[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSets[6].dstSet = model.descriptorSets[i];
            writeDescriptorSets[6].dstBinding = 6;
            writeDescriptorSets[6].dstArrayElement = 0;
            writeDescriptorSets[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSets[6].descriptorCount = MAX_NUM_MODELS;
            writeDescriptorSets[6].pBufferInfo = nullptr;
            writeDescriptorSets[6].pImageInfo = model.albedoDescriptorImages[i].data();
            vkUpdateDescriptorSets(engine.logicalDevice, 6, writeDescriptorSets.data(), 0, NULL);
        }
    }
};
