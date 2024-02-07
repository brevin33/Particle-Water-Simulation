#pragma once
#ifdef VULKANRENDERER

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <pch.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <VulkanRenderer/VulkanEXT.h>
#include <stb_image.h>
#include <Mesmerize/Renderer.h>


namespace MZ {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
        }

        bool uniqueFamily(uint32_t family) {
            return family != graphicsFamily && family != presentFamily && family != computeFamily;
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    enum BufferMappingType {
        NoMapping,
        Mapping,
        PersitantMapping,
    };

    GLFWwindow* window;
    bool framebufferResized = false;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VmaAllocator allocator;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::array<std::vector<VkFramebuffer>,2> swapChainFramebuffers;

    VkRenderPass renderPass;

    VkRenderPass defferedRenderPass;
    VkPipelineLayout defferedPipelineLayout;
    VkPipeline defferedGraphicsPipeline = nullptr;
    VkDescriptorSetLayout defferedDescriptorSetLayout;
    VkDescriptorPool defferedDescriptorPool;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> defferedDescriptorSets;

    struct DefferedParams {
        std::string fragShader; TextureID* textureIDs; uint32_t numTextureIDs; UniformBufferID* bufferIDs; uint32_t numBuffers;
    };
    DefferedParams defferedParams;


    VkCommandPool commandPool;
    VkCommandPool computeCommandPool;

    VkSampler imageSampler;

    std::array<VkCommandBuffer,MAX_FRAMES_IN_FLIGHT> commandBuffers;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> computeCommandBuffers;


    std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT> drawCommandBuffer;
    std::array<VmaAllocation, MAX_FRAMES_IN_FLIGHT> drawCommandBufferMemory;
    std::array<VmaAllocationInfo, MAX_FRAMES_IN_FLIGHT> drawCommandBufferMapped;

    VkImage depthImage;
    VmaAllocation depthImageMemory;
    VkImageView depthImageView;

    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> computeFinishedSemaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> computeInFlightFences;
    uint32_t currentFrame = 0;

    std::vector<VkImage> colorImage;
    std::vector<VmaAllocation> colorImageMemory;
    std::vector<VkImageView> colorImageView;

    std::vector<VkImage> colorImageMsaaOut;
    std::vector<VmaAllocation> colorImageMemoryMsaaOut;
    std::vector<VkImageView> colorImageViewMsaaOut;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    std::vector<TextureID> unfilledTextureIDs;
    std::vector<MaterialID> unfilledMaterialIDs;
    std::vector<ShaderID> unfilledShaderIDs;
    std::vector<VertexBufferID> unfilledVertexBufferIDs;
    std::vector<UniformBufferID> unfilledUniformBufferIDs;
    std::vector<IndexBufferID> unfilledIndexBufferIDs;
    std::vector<ComputeShaderID> unfilledComputeShaderIDs;
    std::vector<std::vector<ComputeID>> unfilledComputeIDs;
    std::vector<RenderObjectID> unfilledRenderObjectIDs;

    std::vector<UniformBufferID> mutUniformBuffers;
    std::vector<VertexBufferID> mutVertexBuffers;
    std::vector<IndexBufferID> mutIndexBuffers;

    std::vector<UniformBufferID> mutGPUUniformBuffers;
    std::vector<VertexBufferID> mutGPUVertexBuffers;
    std::vector<IndexBufferID> mutGPUIndexBuffers;
    std::vector<TextureID> mutGPUTextures;

    std::vector<UniformBufferID> constUniformBuffers;
    std::vector<VertexBufferID> constVertexBuffers;
    std::vector<IndexBufferID> constIndexBuffers;
    std::vector<TextureID> constTextures;

    // should index by RenderObjectID
    struct RenderObject {
        bool shouldDraw;
        MaterialID material;
        VertexBufferID vertexBuffer;
        IndexBufferID indexBuffer;
        VertexBufferID instanceBuffer;
        uint8_t numVertexBuffers;
    };
    std::vector<RenderObject> renderObjects;

    // should index by ComputeID
    struct Compute {
        bool shouldRun;
        ComputeShaderID shaderID;
        uint32_t x;
        uint32_t y;
        uint32_t z;
        std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
    };
    std::vector<std::vector<Compute>> computes;
    std::vector<VkEvent> computestageEvents;

    //should index by ComputeShaderID
    std::vector<VkDescriptorSetLayout> computeDescriptorSetLayout;
    std::vector<VkPipelineLayout> computePipelineLayout;
    std::vector<VkPipeline> computePipeline;
    std::vector<VkDescriptorPool> computeDescriptorPool;


    // should index by UniformBufferID
    std::vector<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>> uniformBuffers;
    std::vector<std::array<VmaAllocation, MAX_FRAMES_IN_FLIGHT>> uniformBuffersMemory;
    std::vector<std::array<VmaAllocationInfo,MAX_FRAMES_IN_FLIGHT>> uniformBuffersMapped;
    std::vector<void*> uniformBufferData;
    std::vector<uint32_t> uniformBuffersSize;
    std::vector<uint8_t> uniformUpToDate;

    // should index by VertexBufferID
    std::vector<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>> vertexBuffers;
    std::vector<std::array<VmaAllocation, MAX_FRAMES_IN_FLIGHT>> vertexBufferMemorys;
    std::vector<std::array<VmaAllocationInfo, MAX_FRAMES_IN_FLIGHT>> vertexBuffersMapped;
    std::vector<void*> vertexBufferData;
    std::vector<uint32_t> vertexNumInstances;
    std::vector<uint64_t> vertexBuffersSize;
    std::vector<uint8_t> vertexUpToDate;

    // should index by IndexBufferID
    std::vector<std::array<VkBuffer, MAX_FRAMES_IN_FLIGHT>> indexBuffers;
    std::vector<std::array<VmaAllocation, MAX_FRAMES_IN_FLIGHT>> indexBufferMemorys;
    std::vector<std::array<VmaAllocationInfo, MAX_FRAMES_IN_FLIGHT>> indexBuffersMapped;
    std::vector<void*> indexBufferData;
    std::vector<uint32_t> indexBuffersSize;
    std::vector<uint32_t> indexNumIndices;
    std::vector<uint8_t> indexUpToDate;

    //should index by MaterialID
    std::vector<ShaderID> materialShaderIDs;
    std::vector<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> materialDescriptorSets;

    // should index by ShaderID
    std::vector<VkPipelineLayout> shaderPipelineLayouts;
    std::vector<VkPipeline> shaderGraphicsPipelines;
    std::vector<VkDescriptorSetLayout> shaderDescriptorSetLayouts;
    std::vector<VkDescriptorPool> shaderDescriptorPools;

    // should index by TextureID
    std::vector<std::array<VkImage, MAX_FRAMES_IN_FLIGHT>> textureImages;
    std::vector<std::array<VmaAllocation, MAX_FRAMES_IN_FLIGHT>> textureImageMemorys;
    std::vector<std::array<VkImageView, MAX_FRAMES_IN_FLIGHT>> textureImageViews;
    std::vector<VkImageLayout> textureImageLayout;


    bool hasCubemap = false;
    ShaderID skyboxShader;
    MaterialID skyboxMaterial;
    VertexBufferID skyboxVertexBuffer;
    IndexBufferID skyboxIndexBuffer;


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
    };

    void setupNoDefaults(GLFWwindow* window, int numGColorBuffers, std::string pathToRendererDir);

    void drawObjects(VkCommandBuffer& commandBuffer, uint32_t renderFrame);

    void recreateSwapChain();

    void cleanupSwapChain();

    void createTextureSampler(VkSampler& textureSampler);
        
    void createTextureImage(void* imageData, uint32_t texWidth, uint32_t texHeight, VmaAllocation& textureImageMemory, VkImage& textureImage, VkImageView& textureImageView, bool createMipMaps, ImageFormat imageFormat, bool gpuSide, VkImageLayout finalLayout, bool cube);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t pixelSize, bool cube);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkImageLayout finalLayout, bool cube = false);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, bool cube = false);

    bool hasStencilComponent(VkFormat format);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t renderFrame);

    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);

    void createDrawCommandBuffer();

    void createDefferedDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, int numTextures, uint32_t numBuffers);

    void createDescriptorPool(VkDescriptorPool& descriptorPool, uint32_t poolSize, int numTextures, uint32_t numBuffers, uint32_t numStorageBuffers, uint32_t numStorageIamges, bool hasDrawCommandBuffer, bool isDefferedShader);

    void createDescriptorSets(std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& descriptorSets, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers,
        UniformBufferID* storageUniforms, bool* storageUniformsLastFrame, uint32_t numStorageUniforms, VertexBufferID* storageVertex, bool* storageVertexLastFrame, uint32_t numStorageVertex, IndexBufferID* storageIndex, bool* storageIndexLastFrame,
        uint32_t numStorageIndex, TextureID* storageTextureIDs, bool* storageTextureLastFrame, uint32_t numstorageTextures, bool hasDrawCommandBuffer, bool isDefferedShader);

    void createGraphicsPipline(std::string vertShaderPath, std::string fragShaderPath, VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline, VkDescriptorSetLayout& descriptorSetLayout, VertexValueType* vertexValues, uint32_t numVertexValues, VertexValueType* InstanceTypes, uint32_t numInstanceTypes,
        std::string tessContorlShaderPath, std::string tessEvaluationShaderPath, CullMode cullMode);

    void createDefferdGraphicsPipline(std::string fragShaderPath);

    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, ShaderStages* textureAccess, int numTextures, ShaderStages* bufferAccess, uint32_t numBuffers);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& bufferMemory, BufferMappingType mapping, VmaAllocationInfo* allocationInfo = nullptr);

    void createComputeDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, uint32_t numUbos, uint32_t numImages, uint32_t numStorageBuffer, uint32_t numStorageImages, bool hasDrawCommandBuffer);

    void createComputePipeline(std::string computeShaderFilePath, VkDescriptorSetLayout& descriptorSetLayout, VkPipelineLayout& pipleineLayout, VkPipeline& pipeline);

    VkCommandBuffer beginSingleTimeCommands();

    void createGPUSideOnlyBuffer(void* data, uint64_t dataSize, VkDeviceSize bufferSize, VkBuffer& buffer, VmaAllocation& bufferMemory, VkBufferUsageFlags useageFlags);

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void createColorMsaaOutResources(int numColorResorces);

    std::array<uint32_t, 3> getOffsetVertexValue(VertexValueType vertexValue);

    VkVertexInputBindingDescription getBindingDescription(VertexValueType* VertexValues, uint32_t numVertexValues, VkVertexInputRate inputRate, uint32_t binding);

    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(VertexValueType* VertexValues, uint32_t numVertexValues, uint32_t binding, uint32_t layoutOffset);

    static std::vector<char> readFile(const std::string& filename);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    uint32_t imageFormatSize(ImageFormat imageFormat);

    ComputeID getNewComputeID(uint16_t computePass);

    RenderObjectID getNewRenderObjectID();

    MaterialID getNewMaterialID();

    ShaderID getNewShaderID();

    VertexBufferID getNewVertexBufferID();

    UniformBufferID getNewUniformBufferID();

    IndexBufferID getNewIndexBufferID();

    ComputeShaderID getNewComputeShaderID();

    TextureID getNewTextrueID();

    void createSyncObjects();

    void createVmaAllocator();

    void createCommandBuffers();

    void createFramebuffers();

    void createCommandPool();

    void createColorResources(int numColorResorces);

    void createDepthResources();

    VkFormat findDepthFormat();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory, uint32_t arrayLayers = 1, bool cube = false);

    void createRenderPass(int numColorAttachments);

    void createDefferedRenderPass();

    void createComputeCommandBuffers();

    void createSwapChain();

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    void createImageViews();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, bool cube);

    void createLogicalDevice();

    void pickPhysicalDevice();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    bool isDeviceSuitable(VkPhysicalDevice device);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void setupDebugMessenger();

    void createSurface();

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    int64_t ratePhysicalDevice(VkPhysicalDevice phDevice);

    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();

    void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

}



#endif 