#pragma once
#include "Data.h"
#include "Includes.h"


struct ImageInfo{
    VkImage image;
    VkDeviceMemory imageMemory;
};

struct BufferInfo{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
};

struct HostVisibleBuffer{
    void * memoryPointer;
    BufferInfo bufferPack;
};

struct WindowInfo{
    float xScale;
    int fbWidth;
    int fbHeight;
    int windowWidth;
    int windowHeight;
    GLFWwindow * handle;
};

struct Mesh{
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> textureCoordinates;
};

struct LightInfo{
    LightConfig config;
    glm::vec4 boundingSphere;
    float maxLightIntensityComponent;
};

struct CameraInfo{
    glm::vec4 up;
    glm::vec4 right;
    float fovRadians;
    float angle = 0.0f;
    glm::vec4 direction;
    CameraConfig config;
    glm::mat4 viewMatrix;
    VkExtent2D resolution;
    glm::mat4 projectionMatrix;
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 lookAroundRotation = glm::mat4(1.0f);
};

struct ModelShaderUniform{
    struct Vertex {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };
    struct alignas(16) Fragment {
        uint32_t numLights;
        float nearClippingPlane;
        float farClippingPlane;
        float lightCullingEnabled;
        glm::uvec4 heatMap_numCellsPerTile_albedo_normal;
        glm::vec2 screenResolution;
        glm::uvec2 numTiles2D;
        glm::vec4 cameraPosition_isDepth;
        std::array<glm::vec4, MAX_NUM_LIGHTS> lightPositions;
        std::array<glm::vec4, MAX_NUM_LIGHTS> lightColors;
    };
};

struct TextureInfo{
    VkSampler sampler = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
};

struct ModelInfo{
    uint32_t numDrawCommands;
    VkPipeline graphicsPipeline;
    uint32_t totalNumTriangles = 0;
    VkDescriptorPool descriptorPool;
    VkPipelineLayout pipelineLayout;
    ModelShaderUniform::Vertex vsData;
    BufferInfo textureCoordinateBuffer;
    ModelShaderUniform::Fragment fsData;
    std::vector<VkBuffer> vertexBuffers;
    BufferInfo vertexBuffer, normalBuffer;
    std::vector<TextureInfo> albedoTextures;
    glm::vec3 max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    glm::vec3 min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    BufferInfo indexBuffer, drawCommandsBuffer;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDeviceSize> vertexBufferOffsets;
    std::vector<HostVisibleBuffer> vertexShaderUniformBuffers;
    std::vector<HostVisibleBuffer> fragmentShaderUniformBuffers;
    std::vector<VkDescriptorBufferInfo> fragmentShaderUniformDescriptorBuffers;
    std::vector<std::vector<VkDescriptorImageInfo>> albedoDescriptorImages;
    std::vector<VkDescriptorBufferInfo> vertexShaderUniformDescriptorBuffers;
};


struct DepthPrepassShaderUniform{
    struct Vertex{
        alignas(16) glm::mat4 cameraViewMatrix;
        alignas(16) glm::mat4 cameraProjectionMatrix;
    };
};

struct DepthPrepassInfo{
    VkSampler sampler;
    VkRenderPass renderPass;
    std::vector<VkFence> fences;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkBuffer> vertexBuffers;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> frameBuffers;
    DepthPrepassShaderUniform::Vertex vsData;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkSemaphore> finishedSemaphores;
    std::vector<VkDeviceSize> vertexBufferOffsets;
    std::vector<VkDescriptorImageInfo> descriptorImages;
    std::vector<HostVisibleBuffer> vertexShaderUniformBuffers;
};

struct LightCullingPrepassUniformVariables{
    float cameraNearPlane;
    float cameraFarPlane;
    glm::uvec2 numTiles2D;
    unsigned int numCellsPerTile;
};

struct LightCullingPrepassInfo{
    bool numCellsPerTileChange = false;
    unsigned int numCellsPerTile = 32;
    unsigned int proposedNumCellsPerTile = 32;
    glm::uvec2 numTiles2D;
    VkPipeline computePipeline;
    unsigned int totalNumTiles;
    std::vector<VkFence> fences;
    VkPipelineLayout pipelineLayout;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> finishedSemaphores;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<HostVisibleBuffer> uniformBuffers;
    LightCullingPrepassUniformVariables uniformData;
    std::vector<VkDescriptorBufferInfo> tileLightCullingPrepassDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> tilesGeometryBitMaskDescriptorBuffers;
    unsigned int minNumCellsPerTile = 1;
    unsigned int maxNumCellsPerTile = 32;
};

struct LightCullingUniformVariables{
    float nearClippingPlane;
    float farClippingPlane;
    glm::uvec2 numTiles2D;
    unsigned int numCellsPerTile;
};

struct LightCullingInfo{
    std::vector<BufferInfo> tileLightIndicesBuffers;
    std::vector<BufferInfo> tileLightCountBuffers;
    std::vector<BufferInfo> maxLightCountBuffers;
    glm::uvec2 numTiles2D;
    unsigned int numLights;
    unsigned int totalNumTiles;
    VkPipeline computePipeline;
    std::vector<VkFence> fences;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    LightCullingUniformVariables uniformData;
    std::vector<glm::vec4> tilesFrustumPlanes;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkSemaphore> finishedSemaphores;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<HostVisibleBuffer> uniformBuffers;
    std::vector<glm::vec4> lightBoundingSpheresViewSpace;
    std::vector<HostVisibleBuffer> tilesFrustumPlanesBuffers;
    std::vector<HostVisibleBuffer> lightBoundingSphereViewSpaceBuffers;
    std::vector<VkDescriptorBufferInfo> tileLightCountDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> maxLightCountDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> tileLightIndicesDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> lightBoundingSphereViewSpaceDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> tilesFrustumPlanesDescriptorBuffers;
    std::vector<VkDescriptorBufferInfo> uniformDescriptorBuffers;
};

struct SceneInfo{
    bool displayDepth = false;
    ModelInfo model;
    int selectedLight;
    CameraInfo camera;
    bool displayAlbedo = false;
    bool displayNormal = false;
    bool heatMap = false;
    VkRenderPass renderPass;
    VkExtent2D screenResolution;
    VkPipeline graphicsPipeline;
    bool applyToAllLights = false;
    DepthPrepassInfo depthPrepass;
    std::vector<LightInfo> lights;
    LightCullingInfo lightCulling;
    std::vector<VkRect2D> scissors;
    bool lightCullingRecorded = false;
    bool lightCullingEnabled = true;
    VkPipelineLayout pipelineLayout;
    std::vector<VkViewport> viewports;
    std::vector<std::string> lightNames;
    std::vector<VkFence> inflightFences;
    std::vector<VkClearValue> clearValues;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<const char *> lightNamesPtr;
    LightCullingPrepassInfo lightCullingPrepass;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> renderFinishedSemaphores;
};

struct UIInfo{
    VkRect2D scissor;
    VkViewport viewport;
    VkRenderPass renderPass;
    std::vector<VkFence> inflightFences;
    std::vector<VkClearValue> clearValues;
    std::vector<VkFramebuffer> frameBuffers;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> renderFinishedSemaphores;
};

struct SwapChainInfo{
    VkExtent2D extent;
    uint32_t imageCount;
    VkSwapchainKHR handle;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkSurfaceFormatKHR swapChainSurface;
    VkPresentModeKHR swapChainPresentMode;
};

struct PhysicalDeviceInfo{
    uint32_t queueIndex;
    VkPhysicalDevice handle;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct EngineInfo{
    uint32_t imageIndex;
    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkFormat depthFormat;
    VkDevice logicalDevice;
    SwapChainInfo swapChain;
    bool isQueueBusy = false;
    VkInstance vulkanInstance;
    VkSurfaceKHR windowSurface;
    PhysicalDeviceInfo physicalDevice;
    std::vector<VkSemaphore> imageAvailableSemaphores;
};

struct ApplicationInfo{
    UIInfo ui;
    EngineInfo engine;
    WindowInfo window;
    uint32_t frame = 0;
    int selectedScene = 0;
    int nextSelectedScene = 0;
    std::vector<SceneInfo> scenes;
    std::vector<const char *> sceneNamesPtr;
    std::vector<std::string> sceneNames;
};
