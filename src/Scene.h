#pragma once
#include "Camera.h"
#include "Model.h"
#include "UI.h"
#include "LightCulling.h"
#include "LightCullingPrepass.h"
#include "Engine.h"
#include "Light.h"
#include "Camera.h"



class Scene {
    
public:
    
    static void Build(SceneInfo& scene, EngineInfo& engine, const SceneConfig& sceneConfig)
    {
        Camera::Build(engine, scene.camera, scene.model, scene.depthPrepass, sceneConfig.cameraConfig, scene.displayDepth);
        
        Model::Load(engine, scene.model, scene.depthPrepass, sceneConfig.models);
        
        Light::Build(engine, scene);
        
        scene.clearValues = std::vector<VkClearValue>{
            VkClearValue{
                .color = {0.0f, 0.0f, 0.0f, 1.0f}
            },
            VkClearValue{
                .depthStencil = {1.0f, 0}
            }
        };
        
        scene.viewports = std::vector<VkViewport>{
            VkViewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(engine.swapChain.extent.width),
                .height = static_cast<float>(engine.swapChain.extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            }
        };
        
        scene.scissors = std::vector<VkRect2D>{
            VkRect2D{
                .offset = { 0, 0 },
                .extent = engine.swapChain.extent
            }
        };

        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        std::vector<VkAttachmentDescription> attachmentDescriptions(2);
        attachmentDescriptions[0].format = engine.swapChain.swapChainSurface.format;
        attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        attachmentDescriptions[1].format = engine.depthFormat;
        attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.colorAttachmentCount = 1;
        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
        
        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO ;
        info.attachmentCount = 2;
        info.pAttachments = attachmentDescriptions.data();
        info.pSubpasses = &subpassDescription;
        info.subpassCount = 1;
        info.pDependencies = &subpassDependency;
        info.dependencyCount = 1;
        if(vkCreateRenderPass(engine.logicalDevice, &info, nullptr, &scene.renderPass) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO CREATE RENDER PASS");

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
        scene.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        if(vkAllocateCommandBuffers(engine.logicalDevice, &commandBufferAllocateInfo, scene.commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE COMMAND BUFFERS\n");

        scene.inflightFences.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.pNext = nullptr;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            if(vkCreateFence(engine.logicalDevice, &fenceCreateInfo, nullptr, &scene.inflightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FENCES\n");
        }

        scene.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT) ;
        for( int i = 0 ; i < MAX_FRAMES_IN_FLIGHT ; i++ ){
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if(vkCreateSemaphore(engine.logicalDevice, &info, nullptr, &scene.renderFinishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE SEMAPHORES");
        }
        
        DepthPrepass::Build(engine, scene.depthPrepass);
        
        LightCullingPrepass::Build(engine, scene.depthPrepass, scene.lightCullingPrepass);
        
        LightCulling::Build(engine, scene.lightCulling, scene.lightCullingPrepass, scene.lights);
                
        Model::Build(engine, scene.model, scene.renderPass, scene.lightCulling, scene.lightCullingPrepass, scene.lights, scene.depthPrepass, scene.lightCullingEnabled, scene.heatMap, scene.displayAlbedo, scene.displayNormal);

        scene.frameBuffers.resize(engine.swapChain.imageViews.size()) ;
        for(size_t i = 0; i < engine.swapChain.imageViews.size(); i++){
            std::vector<VkImageView> imageViews{
                engine.swapChain.imageViews[i],
                scene.depthPrepass.imageViews[i]
            };
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = scene.renderPass;
            framebufferCreateInfo.attachmentCount = 2;
            framebufferCreateInfo.pAttachments = imageViews.data();
            framebufferCreateInfo.width = engine.swapChain.extent.width;
            framebufferCreateInfo.height = engine.swapChain.extent.height;
            framebufferCreateInfo.layers = 1;
            if(vkCreateFramebuffer(engine.logicalDevice, &framebufferCreateInfo, nullptr, &scene.frameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FRAMEBUFFERS");
        }
    }

    static void Record(SceneInfo& scene, EngineInfo& engine, uint32_t frame)
    {
        if(scene.lightCullingPrepass.numCellsPerTileChange){
            scene.lightCullingPrepass.numCellsPerTileChange = false;
            scene.lightCullingPrepass.numCellsPerTile = scene.lightCullingPrepass.proposedNumCellsPerTile;
            scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.g = scene.lightCullingPrepass.numCellsPerTile;
            scene.lightCullingPrepass.uniformData.numCellsPerTile = scene.lightCullingPrepass.numCellsPerTile;
            scene.lightCulling.uniformData.numCellsPerTile = scene.lightCullingPrepass.numCellsPerTile;
            LightCulling::UpdateNumCellsPerTile(engine, scene.lightCullingPrepass, scene.lightCulling, scene.model);
        }
        
        DepthPrepass::Record(engine, scene.depthPrepass, scene.model, scene.camera, frame);
        
        if(scene.lightCullingEnabled){
            LightCullingPrepass::Record(engine, scene.camera, scene.lightCullingPrepass, frame);
            LightCulling::Record(engine, scene, frame);
            scene.lightCullingRecorded = true;
        }
        
        vkWaitForFences(engine.logicalDevice, 1, &scene.inflightFences[frame], VK_TRUE, UINT64_MAX);
        
        vkResetFences(engine.logicalDevice, 1, &scene.inflightFences[frame]);
        
        if(vkResetCommandBuffer(scene.commandBuffers[frame], 0) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO RESET COMMAND BUFFER\n");
        
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(scene.commandBuffers[frame], &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BEGIN RECORDING COMMAND BUFFER\n");
        
        VkRenderPassBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = scene.renderPass,
            .framebuffer = scene.frameBuffers[frame],
            .renderArea.offset = {0, 0},
            .renderArea.extent = engine.swapChain.extent,
            .clearValueCount = static_cast<uint32_t>(scene.clearValues.size()),
            .pClearValues = scene.clearValues.data()
        };
        vkCmdBeginRenderPass(scene.commandBuffers[frame], &info, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdSetViewport(scene.commandBuffers[frame], 0, static_cast<uint32_t>(scene.viewports.size()), scene.viewports.data());

        vkCmdSetScissor(scene.commandBuffers[frame], 0, static_cast<uint32_t>(scene.scissors.size()), scene.scissors.data()) ;

        Model::Render(engine.logicalDevice, scene.model, scene.commandBuffers[frame], frame);
        
        vkCmdEndRenderPass(scene.commandBuffers[frame]);
        
        if(vkEndCommandBuffer(scene.commandBuffers[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO END RECORDING COMMAND BUFFER\n");
    }

    static void Submit(SceneInfo& scene, EngineInfo& engine, uint32_t frame)
    {
        DepthPrepass::Submit(scene.depthPrepass, engine, frame);
        
        std::vector<VkSemaphore> waitSemaphores;
        waitSemaphores.push_back(engine.imageAvailableSemaphores[frame]);
        std::vector<VkPipelineStageFlags> stageFlags;
        stageFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        if(scene.lightCullingRecorded){
            waitSemaphores.push_back(scene.lightCulling.finishedSemaphores[frame]);
            stageFlags.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            LightCullingPrepass::Submit(engine, scene.depthPrepass, scene.lightCullingPrepass, frame);
            LightCulling::Submit(engine, scene.lightCulling, scene.lightCullingPrepass, frame);
            scene.lightCullingRecorded = false;
        }
        else{
            waitSemaphores.push_back(scene.depthPrepass.finishedSemaphores[frame]);
            stageFlags.push_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        }
        
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &scene.commandBuffers[frame];
        info.waitSemaphoreCount = 2;
        info.pWaitSemaphores = waitSemaphores.data();
        info.pWaitDstStageMask = stageFlags.data();
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &scene.renderFinishedSemaphores[frame];
        
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, scene.inflightFences[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
    }
};
