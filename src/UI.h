#pragma once
#include "Engine.h"
#include "Scene.h"
#include "Light.h"



class UI{
    
public:
    
    static void Build(UIInfo& ui, EngineInfo& engine, WindowInfo& window)
    {
        ui.clearValues = std::vector<VkClearValue>{
            VkClearValue{
                .color = {0.0f, 0.0f, 0.0f, 1.0f}
            },
            VkClearValue{
                .depthStencil = {1.0f, 0}
            }
        };
        
        ui.viewport = VkViewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(engine.swapChain.extent.width),
            .height = static_cast<float>(engine.swapChain.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        
        ui.scissor = VkRect2D{
            .offset = { 0, 0 },
            .extent = engine.swapChain.extent
        };
        
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = engine.swapChain.swapChainSurface.format;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pDepthStencilAttachment = nullptr;
        
        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO ;
        info.attachmentCount = 1;
        info.pAttachments = &attachmentDescription;
        info.pSubpasses = &subpassDescription;
        info.subpassCount = 1;
        info.pDependencies = &subpassDependency;
        info.dependencyCount = 1;
        if(vkCreateRenderPass(engine.logicalDevice, &info, nullptr, &ui.renderPass) != VK_SUCCESS)
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
        ui.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        if(vkAllocateCommandBuffers(engine.logicalDevice, &commandBufferAllocateInfo, ui.commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO ALLOCATE COMMAND BUFFERS\n");

        ui.inflightFences.resize(MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.pNext = nullptr;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            if(vkCreateFence(engine.logicalDevice, &fenceCreateInfo, nullptr, &ui.inflightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FENCES\n");
        }
        
        ui.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT) ;
        for( int i = 0 ; i < MAX_FRAMES_IN_FLIGHT ; i++ ){
            VkSemaphoreCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            if(vkCreateSemaphore(engine.logicalDevice, &info, nullptr, &ui.renderFinishedSemaphores[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE SEMAPHORES");
        }

        uint32_t numSwapChainImages = engine.swapChain.imageViews.size();
        ui.frameBuffers.resize(numSwapChainImages);
        for(size_t i = 0; i < numSwapChainImages; i++){
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = ui.renderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = &engine.swapChain.imageViews[i];
            framebufferCreateInfo.width = engine.swapChain.extent.width;
            framebufferCreateInfo.height = engine.swapChain.extent.height;
            framebufferCreateInfo.layers = 1;
            if(vkCreateFramebuffer(engine.logicalDevice, &framebufferCreateInfo, nullptr, &ui.frameBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("\nFAILURE TO CREATE FRAMEBUFFERS");
        }
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        static const ImWchar ranges[] = {0x0020, 0x00FF, 0x2190, 0x21FF, 0};
        io.Fonts->AddFontFromFileTTF("../Resources/assets/Fonts/EpundaSans.ttf", 25.0f, nullptr, ranges);
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard ;
        ImGui::StyleColorsDark();

        ImGui::GetIO().IniFilename = nullptr;
        ImGui_ImplGlfw_InitForVulkan(window.handle, true) ;

        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = engine.vulkanInstance;
        initInfo.PhysicalDevice = engine.physicalDevice.handle;
        initInfo.Device = engine.logicalDevice;
        initInfo.Queue = engine.graphicsQueue;
        initInfo.DescriptorPool = nullptr;
        initInfo.DescriptorPoolSize = 1000;
        initInfo.MinImageCount = engine.swapChain.imageCount;
        initInfo.ImageCount = engine.swapChain.imageCount;
        initInfo.UseDynamicRendering = false;
        initInfo.PipelineInfoMain.RenderPass = ui.renderPass;
        initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        if(!ImGui_ImplVulkan_Init(&initInfo))
            throw std::runtime_error("\nFAILURE TO INITIALIZE IMGUI VULKAN BACKEND\n");
    }
    
    static void Record(UIInfo& ui, SceneInfo& scene, WindowInfo& window, EngineInfo& engine, uint32_t frame, std::vector<const char *>& sceneNamesPtr, int& nextSelectedScene)
    {
        vkWaitForFences(engine.logicalDevice, 1, &ui.inflightFences[frame], VK_TRUE, UINT64_MAX);
        
        vkResetFences(engine.logicalDevice, 1, &ui.inflightFences[frame]) ;
        
        if(vkResetCommandBuffer(ui.commandBuffers[frame], 0) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO RESET COMMAND BUFFER\n");
        
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(ui.commandBuffers[frame], &commandBufferBeginInfo) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO BEGIN RECORDING COMMAND BUFFER\n");
        
        VkRenderPassBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = ui.renderPass,
            .framebuffer = ui.frameBuffers[frame],
            .renderArea.offset = {0, 0},
            .renderArea.extent = engine.swapChain.extent,
            .clearValueCount = static_cast<uint32_t>(ui.clearValues.size()),
            .pClearValues = ui.clearValues.data()
        };
        vkCmdBeginRenderPass(ui.commandBuffers[frame], &info, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdSetViewport(ui.commandBuffers[frame], 0, 1, &ui.viewport);
        
        vkCmdSetScissor(ui.commandBuffers[frame], 0, 1, &ui.scissor) ;
        
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        RenderPerformancePanel(scene, window.windowWidth, window.windowHeight, frame);
        RenderLightPanel(engine, scene, window.windowWidth, window.windowHeight);
        RenderScenePanel(engine, scene, sceneNamesPtr, nextSelectedScene, window.windowWidth, window.windowHeight);
        
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ui.commandBuffers[frame]);
        
        vkCmdEndRenderPass(ui.commandBuffers[frame]);
        
        if(vkEndCommandBuffer(ui.commandBuffers[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO END RECORDING COMMAND BUFFER\n");
    }
            
    static void RenderLightPanel(EngineInfo& engine, SceneInfo& scene, int windowWidth, int windowHeight)
    {
        LightInfo& light = scene.lights[scene.selectedLight];
        
        ImGui::SetNextWindowSizeConstraints(ImVec2(450, 225), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2(windowWidth, windowHeight), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::Begin("Light", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if(ImGui::BeginTable("Table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)){
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("NumLights").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("NumLights");
            
            ImGui::Separator();
            
            if(!scene.applyToAllLights){
                ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Selected Light").x) * 0.5f) + ImGui::GetCursorPosX());
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Selected Light");
                
                ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Position").x) * 0.5f) + ImGui::GetCursorPosX());
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Position");
                
                ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Color").x) * 0.5f) + ImGui::GetCursorPosX());
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Color");
                
                ImGui::Separator();
            }
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Select All").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Select All");
                                    
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Intensity").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Intensity");
            
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Minimum Light Intensity").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Minimum Light Intensity");
            
            ImGui::TableSetColumnIndex(1);
            
            std::string numLights = std::to_string(scene.lights.size());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize(numLights.c_str()).x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(numLights.c_str());
            
            ImGui::Separator();
            
            if(!scene.applyToAllLights){
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - 120.0f) * 0.5f);
                ImGui::SetNextItemWidth(120.0f);
                ImGui::AlignTextToFramePadding();
                ImGui::Combo("##SelectedLight", &scene.selectedLight, scene.lightNamesPtr.data(), (int)scene.lightNamesPtr.size());
                
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                if(ImGui::DragFloat3("##lightPosition", &light.config.position.x, 0.1f, -100.0f, 100.0f, "%.2f"))
                    scene.model.fsData.lightPositions[scene.selectedLight] = light.config.position;
                
                float cursorPosition = ImGui::GetCursorPosX();
                cursorPosition += (ImGui::GetColumnWidth() - 120.0f) * 0.5f;
                ImGui::SetCursorPosX(cursorPosition);
                ImGui::AlignTextToFramePadding();
                if(ImGui::ColorEdit3("##Color", glm::value_ptr(light.config.color))){
                    light.maxLightIntensityComponent = Light::ComputeMaxLightIntensityComponent(light.config.color, light.config.intensity);
                    light.boundingSphere = Light::ComputeBoundingSphere(light.config.position, light.maxLightIntensityComponent, light.config.minLightIntensity);
                    scene.model.fsData.lightColors[scene.selectedLight] = light.config.intensity * light.config.color;
                }

                ImGui::Separator();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
            ImGui::AlignTextToFramePadding();
            ImGui::Checkbox("##applyToAllLights", &scene.applyToAllLights);
                                    
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            if(ImGui::DragFloat("##Intensity", &light.config.intensity, 0.1f, 0.1f, 10.0f, "%.2f")){
                light.maxLightIntensityComponent = Light::ComputeMaxLightIntensityComponent(light.config.color, light.config.intensity);
                light.boundingSphere = Light::ComputeBoundingSphere(light.config.position, light.maxLightIntensityComponent, light.config.minLightIntensity);
                scene.model.fsData.lightColors[scene.selectedLight] = light.config.intensity * light.config.color;
            }
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            if(ImGui::SliderFloat("##minLightIntensity", &light.config.minLightIntensity, 0.01f, light.maxLightIntensityComponent, "%.4f", ImGuiSliderFlags_Logarithmic)){
                light.config.minLightIntensity = std::clamp(light.config.minLightIntensity, 0.01f, light.maxLightIntensityComponent);
                light.boundingSphere = Light::ComputeBoundingSphere(light.config.position, light.maxLightIntensityComponent, light.config.minLightIntensity);
            }
            
            if(scene.applyToAllLights){
                if(ImGui::Button("Apply")){
                    for(int i = 0; i < scene.lights.size(); i++){
                        scene.lights[i].config.color = light.config.color;
                        scene.lights[i].config.intensity = light.config.intensity;
                        scene.lights[i].config.minLightIntensity = light.config.minLightIntensity;
                        scene.lights[i].maxLightIntensityComponent = light.maxLightIntensityComponent;
                        scene.lights[i].maxLightIntensityComponent = light.maxLightIntensityComponent;
                        scene.model.fsData.lightColors[i] = scene.model.fsData.lightColors[scene.selectedLight];
                        
                        scene.lights[i].boundingSphere = Light::ComputeBoundingSphere(scene.lights[i].config.position, scene.lights[i].maxLightIntensityComponent, scene.lights[i].config.minLightIntensity);
                    }
                }
            }
            
            ImGui::EndTable();
        }
        
        ImGui::End();
    }
    
    static void RenderScenePanel(EngineInfo& engine, SceneInfo& scene, std::vector<const char *>& sceneNamesPtr, int& nextSelectedScene, int windowWidth, int windowHeight)
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2(windowWidth, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if(ImGui::BeginTable("Table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)){
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Selected Scene").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Selected Scene");
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Light Culling").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Light Culling");
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Display Albedo").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Display Albedo");
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Display Normal").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Display Normal");
            
            ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Display Depth").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Display Depth");
            
            if(scene.lightCullingEnabled){
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Heat Map").x) * 0.5f) + ImGui::GetCursorPosX());
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Heat Map");
                
                
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Num Cells Per Tile").x) * 0.5f) + ImGui::GetCursorPosX());
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Num Cells Per Tile");
            }
            
            ImGui::TableSetColumnIndex(1);
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - 200.0f) * 0.5f);
            ImGui::SetNextItemWidth(200.0f);
            ImGui::AlignTextToFramePadding();
            ImGui::Combo("##SelectedScene", &nextSelectedScene, sceneNamesPtr.data(), (int)sceneNamesPtr.size());
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
            ImGui::AlignTextToFramePadding();
            if(ImGui::Checkbox("##LightCulling", &scene.lightCullingEnabled)){
                scene.model.fsData.lightCullingEnabled = scene.lightCullingEnabled;
                if(!scene.lightCullingEnabled)
                    scene.heatMap = false;
                    scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.r = scene.heatMap;
                
                scene.displayAlbedo = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.b = scene.displayAlbedo;
                scene.displayNormal = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.a = scene.displayNormal;
                scene.displayDepth = false;
                scene.model.fsData.cameraPosition_isDepth.a = scene.displayDepth;
            }
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
            ImGui::AlignTextToFramePadding();
            if(ImGui::Checkbox("##displayAlbedo", &scene.displayAlbedo)){
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.b = scene.displayAlbedo;
                
                scene.displayNormal = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.a = scene.displayNormal;
                scene.heatMap = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.r = scene.heatMap;
                scene.lightCullingEnabled = false;
                scene.model.fsData.lightCullingEnabled = scene.lightCullingEnabled;
                scene.displayDepth = false;
                scene.model.fsData.cameraPosition_isDepth.a = scene.displayDepth;
            }
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
            ImGui::AlignTextToFramePadding();
            if(ImGui::Checkbox("##displayNormal", &scene.displayNormal)){
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.a = scene.displayNormal;
                                
                scene.displayAlbedo = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.b = scene.displayAlbedo;
                scene.heatMap = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.r = scene.heatMap;
                scene.lightCullingEnabled = false;
                scene.model.fsData.lightCullingEnabled = scene.lightCullingEnabled;
                scene.displayDepth = false;
                scene.model.fsData.cameraPosition_isDepth.a = scene.displayDepth;
            }
            
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
            ImGui::AlignTextToFramePadding();
            if(ImGui::Checkbox("##displayDepth", &scene.displayDepth)){
                scene.model.fsData.cameraPosition_isDepth.a = scene.displayDepth;
                                
                scene.displayAlbedo = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.b = scene.displayAlbedo;
                scene.heatMap = false;
                scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.r = scene.heatMap;
                scene.lightCullingEnabled = false;
                scene.model.fsData.lightCullingEnabled = scene.lightCullingEnabled;
            }
            
            
            if(scene.lightCullingEnabled){
                ImGui::TableSetColumnIndex(1);
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((ImGui::GetColumnWidth() - 50.0f) * 0.5f));
                ImGui::AlignTextToFramePadding();
                if(ImGui::Checkbox("##HeatMap", &scene.heatMap)){
                    scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.r = scene.heatMap;
                    scene.displayAlbedo = false;
                    scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.b = scene.displayAlbedo;
                    scene.displayNormal = false;
                    scene.model.fsData.heatMap_numCellsPerTile_albedo_normal.a = scene.displayNormal;
                    scene.displayDepth = false;
                    scene.model.fsData.cameraPosition_isDepth.a = scene.displayDepth;
                }
                
                if(!scene.displayAlbedo && !scene.displayNormal && !scene.displayDepth){
                    scene.lightCullingEnabled = true;
                    scene.model.fsData.lightCullingEnabled = scene.lightCullingEnabled;
                }
                
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(ImGui::GetColumnWidth());
                if(ImGui::SliderScalar("numCellsPerTile", ImGuiDataType_U32, &scene.lightCullingPrepass.proposedNumCellsPerTile, &scene.lightCullingPrepass.minNumCellsPerTile, &scene.lightCullingPrepass.maxNumCellsPerTile))
                    scene.lightCullingPrepass.numCellsPerTileChange = true;
            }
            
            ImGui::EndTable();
        }
        
        ImGui::End();
    }
    
    static void RenderPerformancePanel(SceneInfo& scene, int windowWidth, int windowHeight, uint32_t frame)
    {
        ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::SetNextWindowPos(ImVec2(0.0f, windowHeight), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        
        if(ImGui::BeginTable("Table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg)){
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("FPS").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("FPS");
            
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Time").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Time");
            
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize("Triangles Count").x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Triangles Count");
            
            ImGui::TableSetColumnIndex(1);
            float fps = ImGui::GetIO().Framerate;
            char fpsText[32];
            snprintf(fpsText, sizeof(fpsText), "%.1f", fps);
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize(fpsText).x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(fpsText);
            
            char timeText[32];
            snprintf(timeText, sizeof(timeText), "%.3f ms", (1000.0f/fps));
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize(timeText).x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(timeText);
            
            char trianglesText[32];
            snprintf(trianglesText, sizeof(trianglesText), "%d", scene.model.totalNumTriangles);
            ImGui::SetCursorPosX(((ImGui::GetColumnWidth() - ImGui::CalcTextSize(trianglesText).x) * 0.5f) + ImGui::GetCursorPosX());
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(trianglesText);
            
            ImGui::EndTable();
        }
        
        ImGui::End();
    }
    
    static void Submit(UIInfo& ui, SceneInfo& scene, WindowInfo& window, uint32_t frame, EngineInfo& engine)
    {
        VkSubmitInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &ui.commandBuffers[frame];
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &scene.renderFinishedSemaphores[frame];
        VkPipelineStageFlags stageFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        info.pWaitDstStageMask = &stageFlag;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &ui.renderFinishedSemaphores[frame];
        
        if(vkQueueSubmit(engine.graphicsQueue, 1, &info, ui.inflightFences[frame]) != VK_SUCCESS)
            throw std::runtime_error("\nFAILURE TO SUBMIT QUEUE\n");
    }
    
    static void Destroy()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
};
