#include <VulkanRenderer/VulkanRenderer.h>
#ifdef VULKANRENDERER

#include <Mesmerize/Renderer.h>
#include <GenericRenderer/DefaultRenderer.h>
#include <GenericRenderer/DefaultPrivate.h>
#include <Mesmerize/Defaults.h>
namespace MZ {


    //----------------------------------------------------------------------------- PUBLIC ---------------------------------------------------- PUBLIC ----------------------------------------------------------------
    void setupNoDefaults(GLFWwindow* w, int numGColorBuffers, std::string pathToRendererDir){
        rendererDir = pathToRendererDir;
        window = w;
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createVmaAllocator();
        createSwapChain();
        createImageViews();
        createRenderPass(numGColorBuffers);
        createDefferedRenderPass();
        createCommandPool();
        createColorResources(numGColorBuffers);
        createDepthResources();
        createColorMsaaOutResources(numGColorBuffers);
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        createDrawCommandBuffer();
        createComputeCommandBuffers();
        createTextureSampler(imageSampler);
    }

    int getRenderWidth() {
        return swapChainExtent.width;
    }

    int getRenderHeight() {
        return swapChainExtent.height;
    }

    void cleanup() {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        for (ShaderID i = (ShaderID)0; i < shaderGraphicsPipelines.size(); i++)
        {
            if (std::find(unfilledShaderIDs.begin(), unfilledShaderIDs.end(), i) == unfilledShaderIDs.end()) {
                deleteResource(i);
            }
        }

        for (ComputeShaderID i = (ComputeShaderID)0; i < computeDescriptorPool.size(); i++)
        {
            if (std::find(unfilledComputeShaderIDs.begin(), unfilledComputeShaderIDs.end(), i) == unfilledComputeShaderIDs.end()) {
                deleteResource(i);
            }
        }

        for (size_t l = 0; l < constTextures.size(); l++)
        {
            TextureID i = constTextures[l];
            if (std::find(unfilledTextureIDs.begin(), unfilledTextureIDs.end(), i) == unfilledTextureIDs.end()) {
                deleteResourceConst(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutGPUTextures.size(); l++)
        {
            TextureID i = mutGPUTextures[l];
            if (std::find(unfilledTextureIDs.begin(), unfilledTextureIDs.end(), i) == unfilledTextureIDs.end()) {
                deleteResourceGPU(i);
                l--;
            }
        }


        vkDestroySampler(device, imageSampler, nullptr);

        for (size_t l = 0; l < mutUniformBuffers.size(); l++)
        {
            UniformBufferID i = mutUniformBuffers[l];
            if (std::find(unfilledUniformBufferIDs.begin(), unfilledUniformBufferIDs.end(), i) == unfilledUniformBufferIDs.end()) {
                deleteResourceCPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutGPUUniformBuffers.size(); l++)
        {
            UniformBufferID i = mutGPUUniformBuffers[l];
            if (std::find(unfilledUniformBufferIDs.begin(), unfilledUniformBufferIDs.end(), i) == unfilledUniformBufferIDs.end()) {
                deleteResourceGPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < constUniformBuffers.size(); l++)
        {
            UniformBufferID i = constUniformBuffers[l];
            if (std::find(unfilledUniformBufferIDs.begin(), unfilledUniformBufferIDs.end(), i) == unfilledUniformBufferIDs.end()) {
                deleteResourceConst(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutVertexBuffers.size(); l++)
        {
            VertexBufferID i = mutVertexBuffers[l];
            if (std::find(unfilledVertexBufferIDs.begin(), unfilledVertexBufferIDs.end(), i) == unfilledVertexBufferIDs.end()) {
                deleteResourceCPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutGPUVertexBuffers.size(); l++)
        {
            VertexBufferID i = mutGPUVertexBuffers[l];
            if (std::find(unfilledVertexBufferIDs.begin(), unfilledVertexBufferIDs.end(), i) == unfilledVertexBufferIDs.end()) {
                deleteResourceGPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < constVertexBuffers.size(); l++)
        {
            VertexBufferID i = constVertexBuffers[l];
            if (std::find(unfilledVertexBufferIDs.begin(), unfilledVertexBufferIDs.end(), i) == unfilledVertexBufferIDs.end()) {
                deleteResourceConst(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutIndexBuffers.size(); l++)
        {
            IndexBufferID i = mutIndexBuffers[l];
            if (std::find(unfilledIndexBufferIDs.begin(), unfilledIndexBufferIDs.end(), i) == unfilledIndexBufferIDs.end()) {
                deleteResourceCPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < mutGPUIndexBuffers.size(); l++)
        {
            IndexBufferID i = mutGPUIndexBuffers[l];
            if (std::find(unfilledIndexBufferIDs.begin(), unfilledIndexBufferIDs.end(), i) == unfilledIndexBufferIDs.end()) {
                deleteResourceGPU(i);
                l--;
            }
        }

        for (size_t l = 0; l < constIndexBuffers.size(); l++)
        {
            IndexBufferID i = constIndexBuffers[l];
            if (std::find(unfilledIndexBufferIDs.begin(), unfilledIndexBufferIDs.end(), i) == unfilledIndexBufferIDs.end()) {
                deleteResourceConst(i);
                l--;
            }
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vmaDestroyBuffer(allocator, drawCommandBuffer[i], drawCommandBufferMemory[i]);
            
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, computeFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
            vkDestroyFence(device, computeInFlightFences[i], nullptr);
        }

        for (size_t i = 0; i < computestageEvents.size(); i++)
        {
            vkDestroyEvent(device, computestageEvents[i], nullptr);
        }

        vkDestroyPipeline(device, defferedGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, defferedPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, defferedDescriptorSetLayout, nullptr);
        vkDestroyDescriptorPool(device, defferedDescriptorPool, nullptr);

        vkDestroyRenderPass(device, defferedRenderPass, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        if (computeCommandPool != commandPool) {
            vkDestroyCommandPool(device, computeCommandPool, nullptr);
        }
        vkDestroyCommandPool(device, commandPool, nullptr);

        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void deleteResourceCPU(UniformBufferID i) {
        unfilledUniformBufferIDs.push_back(i);
        mutUniformBuffers.erase(std::remove(mutUniformBuffers.begin(), mutUniformBuffers.end(), i),mutUniformBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i][j], uniformBuffersMemory[i][j]);
        }
        free(uniformBufferData[i]);
    }

    void deleteResourceCPUSingle(UniformBufferID i) {
        unfilledUniformBufferIDs.push_back(i);
        constUniformBuffers.erase(std::remove(constUniformBuffers.begin(), constUniformBuffers.end(), i), constUniformBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i][j], uniformBuffersMemory[i][j]);
        }
        free(uniformBufferData[i]);
    }

    void deleteResourceConst(UniformBufferID i) {
        unfilledUniformBufferIDs.push_back(i);
        constUniformBuffers.erase(std::remove(constUniformBuffers.begin(), constUniformBuffers.end(), i), constUniformBuffers.end());
        vmaDestroyBuffer(allocator, uniformBuffers[i][0], uniformBuffersMemory[i][0]);
    }
    void deleteResourceGPU(UniformBufferID i) {
        unfilledUniformBufferIDs.push_back(i);
        mutGPUUniformBuffers.erase(std::remove(mutGPUUniformBuffers.begin(), mutGPUUniformBuffers.end(), i), mutGPUUniformBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i][j], uniformBuffersMemory[i][j]);
        }
    }

    void deleteResourceGPUSingle(UniformBufferID i) {
        unfilledUniformBufferIDs.push_back(i);
        constUniformBuffers.erase(std::remove(constUniformBuffers.begin(), constUniformBuffers.end(), i), constUniformBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i][j], uniformBuffersMemory[i][j]);
        }
    }

    void deleteResourceGPU(VertexBufferID i) {
        unfilledVertexBufferIDs.push_back(i);
        mutGPUVertexBuffers.erase(std::remove(mutGPUVertexBuffers.begin(), mutGPUVertexBuffers.end(), i), mutGPUVertexBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, vertexBuffers[i][j], vertexBufferMemorys[i][j]);
        }
    }

    void deleteResourceGPUSingle(VertexBufferID i) {
        unfilledVertexBufferIDs.push_back(i);
        constVertexBuffers.erase(std::remove(constVertexBuffers.begin(), constVertexBuffers.end(), i), constVertexBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, vertexBuffers[i][j], vertexBufferMemorys[i][j]);
        }
    }

    void deleteResourceCPU(VertexBufferID i) {
        unfilledVertexBufferIDs.push_back(i);
        mutVertexBuffers.erase(std::remove(mutVertexBuffers.begin(), mutVertexBuffers.end(), i), mutVertexBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, vertexBuffers[i][j], vertexBufferMemorys[i][j]);
        }
        free(vertexBufferData[i]);
    }

    void deleteResourceCPUSingle(VertexBufferID i) {
        unfilledVertexBufferIDs.push_back(i);
        constVertexBuffers.erase(std::remove(constVertexBuffers.begin(), constVertexBuffers.end(), i), constVertexBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, vertexBuffers[i][j], vertexBufferMemorys[i][j]);
        }
        free(vertexBufferData[i]);
    }

    void deleteResourceConst(VertexBufferID i) {
        unfilledVertexBufferIDs.push_back(i);
        constVertexBuffers.erase(std::remove(constVertexBuffers.begin(), constVertexBuffers.end(), i), constVertexBuffers.end());
        vmaDestroyBuffer(allocator, vertexBuffers[i][0], vertexBufferMemorys[i][0]);
    }
    void deleteResourceConst(IndexBufferID i) {
        unfilledIndexBufferIDs.push_back(i);
        constIndexBuffers.erase(std::remove(constIndexBuffers.begin(), constIndexBuffers.end(), i), constIndexBuffers.end());
        vmaDestroyBuffer(allocator, indexBuffers[i][0], indexBufferMemorys[i][0]);
    }
    void deleteResourceCPU(IndexBufferID i) {
        unfilledIndexBufferIDs.push_back(i);
        mutIndexBuffers.erase(std::remove(mutIndexBuffers.begin(), mutIndexBuffers.end(), i), mutIndexBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, indexBuffers[i][j], indexBufferMemorys[i][j]);
        }
        free(indexBufferData[i]);
    }

    void deleteResourceCPUSingle(IndexBufferID i) {
        unfilledIndexBufferIDs.push_back(i);
        constIndexBuffers.erase(std::remove(constIndexBuffers.begin(), constIndexBuffers.end(), i), constIndexBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, indexBuffers[i][j], indexBufferMemorys[i][j]);
        }
        free(indexBufferData[i]);
    }

    void deleteResourceGPU(IndexBufferID i) {
        unfilledIndexBufferIDs.push_back(i);
        mutGPUIndexBuffers.erase(std::remove(mutGPUIndexBuffers.begin(), mutGPUIndexBuffers.end(), i), mutGPUIndexBuffers.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            vmaDestroyBuffer(allocator, indexBuffers[i][j], indexBufferMemorys[i][j]);
        }
    }

    void deleteResourceGPUSingle(IndexBufferID i) {
        unfilledIndexBufferIDs.push_back(i);
        constIndexBuffers.erase(std::remove(constIndexBuffers.begin(), constIndexBuffers.end(), i), constIndexBuffers.end());
        for (size_t j = 0; j < 1; j++) {
            vmaDestroyBuffer(allocator, indexBuffers[i][j], indexBufferMemorys[i][j]);
        }
    }

    void deleteResourceConst(TextureID i) {
        unfilledTextureIDs.push_back(i);
        constTextures.erase(std::remove(constTextures.begin(), constTextures.end(), i), constTextures.end());
        vmaDestroyImage(allocator, textureImages[i][0], textureImageMemorys[i][0]);
        vkDestroyImageView(device, textureImageViews[i][0], nullptr);
    }
    void deleteResourceGPU(TextureID i) {
        unfilledTextureIDs.push_back(i);
        mutGPUTextures.erase(std::remove(mutGPUTextures.begin(), mutGPUTextures.end(), i), mutGPUTextures.end());
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            vmaDestroyImage(allocator, textureImages[i][j], textureImageMemorys[i][j]);
            vkDestroyImageView(device, textureImageViews[i][j], nullptr);
        }
    }

    void deleteResourceGPUSingle(TextureID i) {
        unfilledTextureIDs.push_back(i);
        constTextures.erase(std::remove(constTextures.begin(), constTextures.end(), i), constTextures.end());
        for (size_t j = 0; j < 1; j++)
        {
            vmaDestroyImage(allocator, textureImages[i][j], textureImageMemorys[i][j]);
            vkDestroyImageView(device, textureImageViews[i][j], nullptr);
        }
    }

    void deleteResource(RenderObjectID i) {
        unfilledRenderObjectIDs.push_back(i);
        renderObjects[i].shouldDraw = false;
    }
    void deleteResource(ComputeID i) {
        unfilledComputeIDs[(i << 16) >> 16].push_back(i);
        computes[(i << 16) >> 16][i >> 16].shouldRun = false;
        vkFreeDescriptorSets(device, computeDescriptorPool[computes[(i << 16) >> 16][i >> 16].shaderID], MAX_FRAMES_IN_FLIGHT, computes[(i << 16) >> 16][i >> 16].descriptorSets.data());
    }
    void deleteResource(ShaderID i) {
        unfilledShaderIDs.push_back(i);
        vkDestroyPipeline(device, shaderGraphicsPipelines[i], nullptr);
        vkDestroyPipelineLayout(device, shaderPipelineLayouts[i], nullptr);
        vkDestroyDescriptorSetLayout(device, shaderDescriptorSetLayouts[i], nullptr);
        vkDestroyDescriptorPool(device, shaderDescriptorPools[i], nullptr);
    }
    void deleteResource(ComputeShaderID i) {
        unfilledComputeShaderIDs.push_back(i);
        vkDestroyPipeline(device, computePipeline[i], nullptr);
        vkDestroyPipelineLayout(device, computePipelineLayout[i], nullptr);
        vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout[i], nullptr);
        vkDestroyDescriptorPool(device, computeDescriptorPool[i], nullptr);
    }
    void deleteResource(MaterialID i) {
        unfilledMaterialIDs.push_back(i);
        vkFreeDescriptorSets(device, shaderDescriptorPools[materialShaderIDs[i]], MAX_FRAMES_IN_FLIGHT, materialDescriptorSets[i].data());
    }

    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, VertexBufferID instanceBuffer) {
        RenderObjectID i = getNewRenderObjectID();
        RenderObject renderObject;
        renderObject.indexBuffer = indexBuffer;
        renderObject.material = material;
        renderObject.vertexBuffer = vertexBuffer;
        renderObject.instanceBuffer = instanceBuffer;
        renderObject.numVertexBuffers = 2;
        renderObject.shouldDraw = true;
        renderObjects[i] = renderObject;

        for(size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            VkDrawIndexedIndirectCommand* drawCommands = (VkDrawIndexedIndirectCommand*)drawCommandBufferMapped[j].pMappedData;
            drawCommands[i].indexCount = indexNumIndices[renderObject.indexBuffer];
            drawCommands[i].instanceCount = vertexNumInstances[renderObject.instanceBuffer];
            drawCommands[i].firstIndex = 0;
            drawCommands[i].vertexOffset = 0;
            drawCommands[i].firstInstance = 0;
        }

        return i;
    }

    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer) {
        RenderObjectID i = getNewRenderObjectID();
        RenderObject renderObject;
        renderObject.indexBuffer = indexBuffer;
        renderObject.material = material;
        renderObject.vertexBuffer = vertexBuffer;
        renderObject.numVertexBuffers = 1;
        renderObject.shouldDraw = true;
        renderObjects[i] = renderObject;

        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            VkDrawIndexedIndirectCommand* drawCommands = (VkDrawIndexedIndirectCommand*)drawCommandBufferMapped[j].pMappedData;
            drawCommands[i].indexCount = indexNumIndices[renderObject.indexBuffer];
            drawCommands[i].instanceCount = 1;
            drawCommands[i].firstIndex = 0;
            drawCommands[i].vertexOffset = 0;
            drawCommands[i].firstInstance = 0;
        }

        return i;
    }

    ComputeID addCompute(ComputeShaderID computeShader, uint32_t xDispatch, uint32_t yDispatch, uint32_t zDispatch, uint16_t computePass, UniformBufferID* UniformBuffers, uint32_t numUniformBuffers,
        TextureID* textures, uint32_t numTextues, UniformBufferID* storageUniforms, bool* storageUniformsLastFrame, uint32_t numStorageUniforms, VertexBufferID* storageVertex, bool* storageVertexLastFrame,
        uint32_t numStorageVertex, IndexBufferID* storageIndex, bool* storageIndexLastFrame, uint32_t numStorageIndex, TextureID* storageTexture, bool* storageTextureLastFrame, uint32_t numStorageTexture, bool hasDrawCommandBuffer){
        ComputeID i = getNewComputeID(computePass);
        Compute compute;
        compute.x = xDispatch;
        compute.y = yDispatch;
        compute.z = zDispatch;
        compute.shaderID = computeShader;
        compute.shouldRun = true;

        createDescriptorSets(compute.descriptorSets, computeDescriptorPool[compute.shaderID], computeDescriptorSetLayout[compute.shaderID], textures, numTextues, UniformBuffers, numUniformBuffers, storageUniforms, storageUniformsLastFrame,
            numStorageUniforms, storageVertex, storageVertexLastFrame, numStorageVertex, storageIndex, storageIndexLastFrame, numStorageIndex, storageTexture, storageTextureLastFrame, numStorageTexture, hasDrawCommandBuffer, false);

        computes[(i << 16) >> 16][i >> 16] = compute;
        return i;

    }

    void setDefferedShader(std::string fragShader, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers) {
        defferedParams.fragShader = fragShader;
        defferedParams.textureIDs = textureIDs;
        defferedParams.numTextureIDs = numTextureIDs;
        defferedParams.bufferIDs = bufferIDs;
        defferedParams.numBuffers = numBuffers;
        if (defferedGraphicsPipeline != nullptr) {
            vkDestroyPipeline(device, defferedGraphicsPipeline, nullptr);
            vkDestroyPipelineLayout(device, defferedPipelineLayout, nullptr);
            vkDestroyDescriptorSetLayout(device, defferedDescriptorSetLayout, nullptr);
            vkDestroyDescriptorPool(device, defferedDescriptorPool, nullptr);
        }
        createDescriptorPool(defferedDescriptorPool, 1, numTextureIDs, numBuffers, 0, 0, false, true);
        createDefferedDescriptorSetLayout(defferedDescriptorSetLayout, numTextureIDs, numBuffers);
        createDefferdGraphicsPipline(fragShader);
        createDescriptorSets(defferedDescriptorSets, defferedDescriptorPool, defferedDescriptorSetLayout, textureIDs, numTextureIDs, bufferIDs, numBuffers, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, false, true);
    }

    void setSkybox(TextureID cubemap) {
        
        if (!hasCubemap) {
            std::vector<uint32_t> skyIndices{
                2, 6, 7,
                2, 3, 7,
                0, 4, 5,
                0, 1, 5,
                0, 2, 6,
                0, 4, 6,
                1, 3, 7,
                1, 5, 7,
                0, 2, 3,
                0, 1, 3,
                4, 6, 7,
                4, 5, 7
            };
            std::vector<float> skyVertices{
                -1, -1,  1,
                 1, -1,  1,
                -1,  1,  1, 
                 1,  1,  1, 
                -1, -1, -1, 
                 1, -1, -1, 
                -1,  1, -1, 
                 1,  1, -1  
            };
            skyboxVertexBuffer = createConstVertexBuffer(skyVertices.data(), skyVertices.size() / 3, sizeof(float) * skyVertices.size());
            skyboxIndexBuffer = createConstIndexBuffer(skyIndices.data(), skyIndices.size() * sizeof(uint32_t));
        }
        else {
            deleteResource(skyboxShader);
            deleteResource(skyboxMaterial);
        }

        ShaderStages vertAccess = SSVert;
        ShaderStages fragAccess = SSFrag;
        VertexValueType vvt = VTfloat3;
        skyboxShader = createShader(rendererDir + "/shaders/skyboxVert.spv", rendererDir + "/shaders/skyboxFrag.spv", 1, &fragAccess, 1, &vertAccess, 1, &vvt, 1, nullptr, 0, NoCull);
        skyboxMaterial = createMaterial(skyboxShader, &cubemap, 1, &mainCameraBuffer, 1);
        hasCubemap = true;
    }

    MaterialID createMaterial(ShaderID shaderID, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers){
        MaterialID i = (MaterialID)materialShaderIDs.size();
        materialShaderIDs.resize(i+1);
        materialDescriptorSets.resize(i+1);

        materialShaderIDs[i] = shaderID;
        createDescriptorSets(materialDescriptorSets[i], shaderDescriptorPools[shaderID], shaderDescriptorSetLayouts[shaderID], textureIDs, numTextureIDs, bufferIDs,  numBuffers, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, false, false);

        return i;
    }

    ShaderID createShader(std::string vertShaderPath, std::string fragShaderPath, uint32_t maxNumberOfMaterial, ShaderStages* textureAccess, uint32_t numTextures, ShaderStages* bufferAccess, uint32_t numBuffers, VertexValueType* VertexValues, uint32_t numVertexValues,
        VertexValueType* InstanceTypes, uint32_t numInstanceTypes, CullMode cullMode) {
        return createShader(vertShaderPath, fragShaderPath, "", "", maxNumberOfMaterial, textureAccess,numTextures, bufferAccess, numBuffers, VertexValues, numVertexValues, InstanceTypes, numInstanceTypes, cullMode);
    }

    ShaderID createShader(std::string vertShaderPath, std::string fragShaderPath, std::string tessalizationControlShaderPath, std::string tessalizationEvaluationShaderPath, uint32_t maxNumberOfMaterial, ShaderStages* textureAccess, uint32_t numTextures,
        ShaderStages* bufferAccess, uint32_t numBuffers, VertexValueType* VertexValues, uint32_t numVertexValues, VertexValueType* InstanceTypes, uint32_t numInstanceTypes, CullMode cullMode)
    {
        ShaderID i = getNewShaderID();

        createDescriptorPool(shaderDescriptorPools[i], maxNumberOfMaterial, numTextures, numBuffers,0,0, false, false);
        createDescriptorSetLayout(shaderDescriptorSetLayouts[i], textureAccess, numTextures, bufferAccess, numBuffers);
        createGraphicsPipline(vertShaderPath, fragShaderPath, shaderPipelineLayouts[i], shaderGraphicsPipelines[i], shaderDescriptorSetLayouts[i], VertexValues, numVertexValues, InstanceTypes, numInstanceTypes, 
            tessalizationControlShaderPath, tessalizationEvaluationShaderPath, cullMode);

        return i;
    }


    ComputeShaderID createComputeShader(std::string computeShaderPath, uint32_t maxNumberOfComputes, uint32_t numUniformBuffers, uint32_t numStaticTextures, uint32_t numStorageBuffers, uint32_t numStorageTextues, bool hasDrawCommandBuffer)
    {
        ComputeShaderID i = getNewComputeShaderID();

        createDescriptorPool(computeDescriptorPool[i], maxNumberOfComputes, numStaticTextures, numUniformBuffers, numStorageBuffers, numStorageTextues, hasDrawCommandBuffer,false);
        createComputeDescriptorSetLayout(computeDescriptorSetLayout[i], numUniformBuffers, numStaticTextures, numStorageBuffers, numStorageTextues, hasDrawCommandBuffer);
        createComputePipeline(computeShaderPath, computeDescriptorSetLayout[i], computePipelineLayout[i], computePipeline[i]);
        return i;
    }


    UniformBufferID createCPUMutUniformBuffer(void* data, uint32_t dataSize, uint32_t bufferSize) {
        UniformBufferID i = getNewUniformBufferID();

        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i][j], uniformBuffersMemory[i][j], PersitantMapping, &uniformBuffersMapped[i][j]);
        }

        uniformUpToDate[i] = 0;
        mutUniformBuffers.push_back(i);
        uniformBufferData[i] = malloc(bufferSize);
        memcpy(uniformBufferData[i], data, dataSize);
        uniformBuffersSize[i] = bufferSize;
        return i;
    }

    UniformBufferID createCPUMutUniformBufferSingle(void* data, uint32_t dataSize, uint32_t bufferSize) {
        UniformBufferID i = getNewUniformBufferID();
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i][0], uniformBuffersMemory[i][0], PersitantMapping, &uniformBuffersMapped[i][0]);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            uniformBuffers[i][j] = uniformBuffers[i][0];
            uniformBuffersMemory[i][j] = uniformBuffersMemory[i][0];
            uniformBuffersMapped[i][j] = uniformBuffersMapped[i][0];
        }

        uniformUpToDate[i] = 0;
        constUniformBuffers.push_back(i);
        uniformBufferData[i] = malloc(bufferSize);
        memcpy(uniformBufferData[i], data, dataSize);
        uniformBuffersSize[i] = bufferSize;
        return i;
    }

    UniformBufferID createGPUMutUniformBuffer(void* data, uint32_t dataSize, uint32_t bufferSize){
        UniformBufferID i = getNewUniformBufferID();

        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createGPUSideOnlyBuffer(data, dataSize, bufferSize, uniformBuffers[i][j], uniformBuffersMemory[i][j], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        }

        uniformUpToDate[i] = 0;
        mutGPUUniformBuffers.push_back(i);
        uniformBuffersSize[i] = bufferSize;
        return i;
    }

    UniformBufferID createGPUMutUniformBufferSingle(void* data, uint32_t dataSize, uint32_t bufferSize) {
        UniformBufferID i = getNewUniformBufferID();
        createGPUSideOnlyBuffer(data, dataSize, bufferSize, uniformBuffers[i][0], uniformBuffersMemory[i][0], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            uniformBuffers[i][j] = uniformBuffers[i][0];
            uniformBuffersMemory[i][j] = uniformBuffersMemory[i][0];
        }

        uniformUpToDate[i] = 0;
        constUniformBuffers.push_back(i);
        uniformBuffersSize[i] = bufferSize;
        return i;
    }

    UniformBufferID createConstUniformBuffer(void* data, uint32_t bufferSize) {
        UniformBufferID i = getNewUniformBufferID();

        uniformBuffersSize[i] = bufferSize;
        createGPUSideOnlyBuffer(data, bufferSize,bufferSize,uniformBuffers[i][0], uniformBuffersMemory[i][0], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            uniformBuffers[i][j] = uniformBuffers[i][0];
            uniformBuffersMemory[i][j] = uniformBuffersMemory[i][0];
        }
        constUniformBuffers.push_back(i);

        return i;
    }

    VertexBufferID createConstVertexBuffer(void* vertices, uint32_t numVertices, uint64_t bufferSize){
        VertexBufferID i = getNewVertexBufferID();

        vertexNumInstances[i] = numVertices;
        vertexBuffersSize[i] = bufferSize;
        createGPUSideOnlyBuffer(vertices, bufferSize, bufferSize, vertexBuffers[i][0], vertexBufferMemorys[i][0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            vertexBuffers[i][j] = vertexBuffers[i][0];
            vertexBufferMemorys[i][j] = vertexBufferMemorys[i][0];
        }
        constVertexBuffers.push_back(i);

        return i;
    }


    VertexBufferID createCPUMutVertexBuffer(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize){
        VertexBufferID i = getNewVertexBufferID();

        vertexNumInstances[i] = numVertices;
        vertexUpToDate[i] = 0;
        vertexBuffersSize[i] = bufferSize;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[i][j], vertexBufferMemorys[i][j], PersitantMapping, &vertexBuffersMapped[i][j]);
        }

        mutVertexBuffers.push_back(i);
        vertexBufferData[i] = malloc(bufferSize);
        memcpy(vertexBufferData[i], vertices, numVertices * vertexSize);
        return i;
    }

    VertexBufferID createCPUMutVertexBufferSingle(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize) {
        VertexBufferID i = getNewVertexBufferID();

        vertexNumInstances[i] = numVertices;
        vertexUpToDate[i] = 0;
        vertexBuffersSize[i] = bufferSize;
        createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[i][0], vertexBufferMemorys[i][0], PersitantMapping, &vertexBuffersMapped[i][0]);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            vertexBuffers[i][j] = vertexBuffers[i][0];
            vertexBufferMemorys[i][j] = vertexBufferMemorys[i][0];
            vertexBuffersMapped[i][j] = vertexBuffersMapped[i][0];
        }

        constVertexBuffers.push_back(i);
        vertexBufferData[i] = malloc(bufferSize);
        memcpy(vertexBufferData[i], vertices, numVertices * vertexSize);
        return i;
    }

    VertexBufferID createGPUMutVertexBuffer(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize) {
        VertexBufferID i = getNewVertexBufferID();

        vertexNumInstances[i] = numVertices;
        vertexUpToDate[i] = 0;
        vertexBuffersSize[i] = bufferSize;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createGPUSideOnlyBuffer(vertices, numVertices * vertexSize, bufferSize, vertexBuffers[i][j], vertexBufferMemorys[i][j], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        }

        mutGPUVertexBuffers.push_back(i);
        return i;
    }

    VertexBufferID createGPUMutVertexBufferSingle(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize) {
        VertexBufferID i = getNewVertexBufferID();

        vertexNumInstances[i] = numVertices;
        vertexUpToDate[i] = 0;
        vertexBuffersSize[i] = bufferSize;
        createGPUSideOnlyBuffer(vertices, numVertices * vertexSize, bufferSize, vertexBuffers[i][0], vertexBufferMemorys[i][0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            vertexBuffers[i][j] = vertexBuffers[i][0];
            vertexBufferMemorys[i][j] = vertexBufferMemorys[i][0];
        }

        constVertexBuffers.push_back(i);
        return i;
    }

    IndexBufferID createConstIndexBuffer(void* indices, uint64_t bufferSize) {
        IndexBufferID i = getNewIndexBufferID();

        indexNumIndices[i] = bufferSize / sizeof(uint32_t);
        indexBuffersSize[i] = bufferSize;
        createGPUSideOnlyBuffer(indices, bufferSize, bufferSize, indexBuffers[i][0], indexBufferMemorys[i][0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            indexBuffers[i][j] = indexBuffers[i][0];
            indexBufferMemorys[i][j] = indexBufferMemorys[i][0];
        }
        constIndexBuffers.push_back(i);

        return i;
    }

    IndexBufferID createCPUMutIndexBuffer(void* indices, uint32_t numIndices, uint64_t bufferSize){
        IndexBufferID i = getNewIndexBufferID();

        indexNumIndices[i] = numIndices;
        indexUpToDate[i] = 0;
        indexBuffersSize[i] = bufferSize;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffers[i][j], indexBufferMemorys[i][j], PersitantMapping, &indexBuffersMapped[i][j]);
        }

        mutIndexBuffers.push_back(i);
        indexBufferData[i] = malloc(bufferSize);
        memcpy(uniformBufferData[i], indices, numIndices * sizeof(uint32_t));
        return i;
    }

    IndexBufferID createCPUMutIndexBufferSingle(void* indices, uint32_t numIndices, uint64_t bufferSize) {
        IndexBufferID i = getNewIndexBufferID();

        indexNumIndices[i] = numIndices;
        indexUpToDate[i] = 0;
        indexBuffersSize[i] = bufferSize;
        createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffers[i][0], indexBufferMemorys[i][0], PersitantMapping, &indexBuffersMapped[i][0]);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            indexBuffers[i][j] = indexBuffers[i][0];
            indexBufferMemorys[i][j] = indexBufferMemorys[i][0];
            indexBuffersMapped[i][j] = indexBuffersMapped[i][0];
        }

        constIndexBuffers.push_back(i);
        indexBufferData[i] = malloc(bufferSize);
        memcpy(uniformBufferData[i], indices, numIndices * sizeof(uint32_t));
        return i;
    }

    IndexBufferID createGPUMutIndexBuffer(void* indices, uint32_t numIndices, uint64_t bufferSize) {
        IndexBufferID i = getNewIndexBufferID();

        indexNumIndices[i] = numIndices;
        indexUpToDate[i] = 0;
        indexBuffersSize[i] = bufferSize;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            createGPUSideOnlyBuffer(indices, numIndices * sizeof(uint32_t), bufferSize, indexBuffers[i][j], indexBufferMemorys[i][j], VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        }

        mutGPUIndexBuffers.push_back(i);
        return i;
    }

    IndexBufferID createGPUMutIndexBufferSingle(void* indices, uint32_t numIndices, uint64_t bufferSize) {
        IndexBufferID i = getNewIndexBufferID();

        indexNumIndices[i] = numIndices;
        indexUpToDate[i] = 0;
        indexBuffersSize[i] = bufferSize;
        createGPUSideOnlyBuffer(indices, numIndices * sizeof(uint32_t), bufferSize, indexBuffers[i][0], indexBufferMemorys[i][0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            indexBuffers[i][j] = indexBuffers[i][0];
            indexBufferMemorys[i][j] = indexBufferMemorys[i][0];
        }

        constIndexBuffers.push_back(i);
        return i;
    }

    TextureID createGPUMutTexture(uint32_t width, uint32_t height, ImageFormat imageFormat) {
        void* data = malloc(width * height * imageFormatSize(imageFormat));
        TextureID i = createGPUMutTexture(data, width, height, imageFormat, false);
        free(data);
        return i;
    }

    TextureID createGPUMutTexture(void* data, uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap) {
        TextureID i = getNewTextrueID();
        textureImageLayout[i] = VK_IMAGE_LAYOUT_GENERAL;
        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            createTextureImage(data, width, height, textureImageMemorys[i][j], textureImages[i][j], textureImageViews[i][j], false, imageFormat, true, VK_IMAGE_LAYOUT_GENERAL, cubemap);
        }
        mutGPUTextures.push_back(i);
        return i;
    }


    TextureID createGPUMutTextureSingle(uint32_t width, uint32_t height, ImageFormat imageFormat) {
        void* data = malloc(width * height * imageFormatSize(imageFormat));
        TextureID i = createGPUMutTextureSingle(data, width, height, imageFormat, false);
        free(data);
        return i;
    }

    TextureID createGPUMutTextureSingle(void* data, uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap) {
        TextureID i = getNewTextrueID();
        textureImageLayout[i] = VK_IMAGE_LAYOUT_GENERAL;
        createTextureImage(data, width, height, textureImageMemorys[i][0], textureImages[i][0], textureImageViews[i][0], false, imageFormat, true, VK_IMAGE_LAYOUT_GENERAL, cubemap);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++) {
            textureImageMemorys[i][j] = textureImageMemorys[i][0];
            textureImages[i][j] = textureImages[i][0];
            textureImageViews[i][j] = textureImageViews[i][0];
        }
        constTextures.push_back(i);
        return i;
    }

    TextureID createConstTexture(void* data, uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap) {
        TextureID i = getNewTextrueID();
        textureImageLayout[i] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        createTextureImage(data, width, height, textureImageMemorys[i][0], textureImages[i][0], textureImageViews[i][0], true, imageFormat, false, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,cubemap);
        for (size_t j = 1; j < MAX_FRAMES_IN_FLIGHT; j++)
        {
            textureImageMemorys[i][j] = textureImageMemorys[i][0];
            textureImages[i][j] = textureImages[i][0];
            textureImageViews[i][j] = textureImageViews[i][0];
        }
        constTextures.push_back(i);

        return i;
    }

    TextureID createConstTexture(std::string textureFilepath) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(textureFilepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        TextureID i = createConstTexture(pixels, texWidth, texHeight, IFSRGBA8,false);
        stbi_image_free(pixels);
        return i;
    }

    void updateCPUMutUniformBuffer(UniformBufferID buffer, void* data, uint32_t dataSize, uint32_t offset) {
        memcpy((void*)((intptr_t)uniformBufferData[buffer] + offset), data, dataSize);
        uniformUpToDate[buffer] = 0;
    }

    void updateCPUMutVertexBuffer(VertexBufferID buffer, void* data, uint32_t dataSize, uint32_t offset) {
        memcpy((void*)((intptr_t)vertexBufferData[buffer] + offset), data, dataSize);
        vertexUpToDate[buffer] = 0;
    }

    void updateCPUMutIndexBuffer(IndexBufferID buffer, void* data, uint32_t dataSize, uint32_t offset) {
        memcpy((void*)((intptr_t)indexBufferData[buffer] + offset), data, dataSize);
        indexUpToDate[buffer] = 0;
    }

    uint32_t getInstanceCount(VertexBufferID vertexBuffer) {
        return vertexNumInstances[vertexBuffer];
    }

    void* getCPUMutUniformBufferData(UniformBufferID buffer) {
        return uniformBufferData[buffer];
    }
    void* getCPUMutVertexBufferData(VertexBufferID buffer) {
        return vertexBufferData[buffer];
    }
    void* getCPUMutIndexBufferData(IndexBufferID buffer) {
        return indexBufferData[buffer];
    }

    //----------------------------------------------------------------------------- PRIVATE ---------------------------------------------------- PRIVATE ----------------------------------------------------------------

    void renderFrame() {

        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        uint32_t renderingFrame = currentFrame;

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        for (size_t j = 0; j < mutUniformBuffers.size(); j++)
        {
            UniformBufferID i = mutUniformBuffers[j];
            if (uniformUpToDate[i] != MAX_FRAMES_IN_FLIGHT)
            {
                memcpy(uniformBuffersMapped[i][renderingFrame].pMappedData, uniformBufferData[i], uniformBuffersSize[i]);
                uniformUpToDate[i]++;
            }
        }
        for (size_t j = 0; j < mutVertexBuffers.size(); j++)
        {
            VertexBufferID i = mutVertexBuffers[j];
            if (vertexUpToDate[i] != MAX_FRAMES_IN_FLIGHT) {
                memcpy(vertexBuffersMapped[i][renderingFrame].pMappedData, vertexBufferData[i], vertexBuffersSize[i]);
                vertexUpToDate[i]++;
            }
        }
        for (size_t j = 0; j < mutIndexBuffers.size(); j++)
        {
            IndexBufferID i = mutIndexBuffers[j];
            if (indexUpToDate[i] != MAX_FRAMES_IN_FLIGHT) {
                memcpy(indexBuffersMapped[i][renderingFrame].pMappedData, indexBufferData[i], indexBuffersSize[i]);
            }
        }

        //compute stuff
        vkWaitForFences(device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &computeInFlightFences[currentFrame]);

        vkResetCommandBuffer(computeCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        recordComputeCommandBuffer(computeCommandBuffers[currentFrame]);


        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

        if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit compute command buffer!");
        };

        // graphics stuff
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex, renderingFrame);

        VkSemaphore waitSemaphores[] = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void drawObjects(VkCommandBuffer& commandBuffer, uint32_t renderFrame) {

        //drawing
        for (size_t i = 0; i < renderObjects.size(); i++)
        {
            RenderObject renderObject = renderObjects[i];
            if (!renderObject.shouldDraw) continue;
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderGraphicsPipelines[materialShaderIDs[renderObject.material]]);

            VkBuffer vertex = vertexBuffers[renderObject.vertexBuffer][renderFrame];
            VkBuffer instance = vertexBuffers[renderObject.instanceBuffer][renderFrame];
            VkBuffer vertexBuffers[] = { vertex, instance };
            VkDeviceSize offsets[] = { 0, 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, renderObject.numVertexBuffers, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, indexBuffers[renderObject.indexBuffer][renderFrame], 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPipelineLayouts[materialShaderIDs[renderObject.material]], 0, 1, &materialDescriptorSets[renderObject.material][renderFrame], 0, nullptr);

            VkDeviceSize indirect_offset = i * sizeof(VkDrawIndexedIndirectCommand);
            uint32_t draw_stride = sizeof(VkDrawIndexedIndirectCommand);

            vkCmdDrawIndexedIndirect(commandBuffer, drawCommandBuffer[renderFrame], indirect_offset, 1, draw_stride);
        }

        if (hasCubemap) {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderGraphicsPipelines[materialShaderIDs[skyboxMaterial]]);

            VkBuffer vertex = vertexBuffers[skyboxVertexBuffer][renderFrame];
            VkBuffer vertexBuffers[] = { vertex };
            VkDeviceSize offsets[] = { 0, 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, indexBuffers[skyboxIndexBuffer][renderFrame], 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPipelineLayouts[materialShaderIDs[skyboxMaterial]], 0, 1, &materialDescriptorSets[skyboxMaterial][renderFrame], 0, nullptr);

            vkCmdDrawIndexed(commandBuffer, indexNumIndices[skyboxIndexBuffer], 1, 0, 0, 0);
        }
    }

    void createVmaAllocator() {

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
        allocatorCreateInfo.physicalDevice = physicalDevice;
        allocatorCreateInfo.device = device;
        allocatorCreateInfo.instance = instance;
        allocatorCreateInfo.pVulkanFunctions = nullptr;

        vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    }

    void recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording compute command buffer!");
        }
        VkMemoryBarrier bar = {};
        bar.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        bar.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        bar.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        for (int computePass = computes.size()-1; computePass >= 0; computePass--) {
            for (size_t i = 0; i < computes[computePass].size(); i++)
            {
                Compute compute = computes[computePass][i];
                if (!compute.shouldRun) continue;
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline[compute.shaderID]);

                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout[compute.shaderID], 0, 1, &compute.descriptorSets[currentFrame], 0, nullptr);

                vkCmdDispatch(commandBuffer, compute.x, compute.y, compute.z);
            }

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &bar, 0, nullptr, 0, nullptr);

        }
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record compute command buffer!");
        }

    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t renderFrame)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[0][imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


        drawObjects(commandBuffer,renderFrame);

        vkCmdEndRenderPass(commandBuffer);

        renderPassInfo.renderPass = defferedRenderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[1][imageIndex];
        renderPassInfo.clearValueCount = 1;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defferedGraphicsPipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defferedPipelineLayout, 0, 1, &defferedDescriptorSets[renderFrame], 0, NULL);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);


        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createColorResources(colorImage.size());
        createDepthResources();
        createColorMsaaOutResources(colorImage.size());
        createFramebuffers();
        setDefferedShader(defferedParams.fragShader, defferedParams.textureIDs, defferedParams.numTextureIDs, defferedParams.bufferIDs, defferedParams.numBuffers);
    }

    void createTextureSampler(VkSampler& textureSampler) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;


        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }

    }


    uint32_t imageFormatSize(ImageFormat imageFormat) {
        switch (imageFormat)
        {
        case MZ::IFSRGBA8:
            return 4;
        case MZ::IFFloat4:
            return sizeof(glm::vec4);
        case MZ::IFFloat3:
            return sizeof(glm::vec3);
        case MZ::IFFloat2:
            return sizeof(glm::vec2);
        case MZ::IFFloat:
            return sizeof(float);
        default:
            break;
        }

    }


    void createTextureImage(void* imageData, uint32_t texWidth, uint32_t texHeight, VmaAllocation& textureImageMemory, VkImage& textureImage, VkImageView& textureImageView, bool createMipMaps, ImageFormat imageFormat, bool gpuSide, VkImageLayout finalLayout, bool cube) {
        
        VkDeviceSize imageSize;
        if(!cube)imageSize = texWidth * texHeight * imageFormatSize(imageFormat);
        else imageSize = texWidth * texHeight * imageFormatSize(imageFormat) * 6;
        uint32_t mipLevels = createMipMaps ? !cube ? static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1 : 1 : 1;

        if (!imageData) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, Mapping);


        void* data;
        vmaMapMemory(allocator, stagingBufferMemory, &data);
        memcpy(data, imageData, static_cast<size_t>(imageSize));
        vmaUnmapMemory(allocator, stagingBufferMemory);

        VkImageUsageFlagBits extraUsage = gpuSide ? VK_IMAGE_USAGE_STORAGE_BIT : (VkImageUsageFlagBits)0;

        createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, (VkFormat)imageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | extraUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, cube ? 6 : 1, cube);

        transitionImageLayout(textureImage, (VkFormat)imageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, cube);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), imageFormatSize(imageFormat), cube);

        vmaDestroyBuffer(allocator,stagingBuffer,stagingBufferMemory);

        generateMipmaps(textureImage, (VkFormat)imageFormat, texWidth, texHeight, mipLevels, finalLayout, cube);

        textureImageView = createImageView(textureImage, (VkFormat)imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, cube);
    }

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, VkImageLayout finalLayout, bool cube) {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = cube ? 6 : 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = cube ? 6 : 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = cube ? 6 : 1;

            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = finalLayout;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = finalLayout;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t pixelSize, bool cube) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        uint32_t layers = cube ? 6 : 1;
        for (int i = 0; i < layers; i++) {
            VkBufferImageCopy region{};
            region.bufferOffset = i * width * height * pixelSize;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = i;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = {
                width,
                height,
                1,
            };

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );
        }
        endSingleTimeCommands(commandBuffer);
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, bool cube) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = cube ? 6 : 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer);
    }

    void createDrawCommandBuffer() {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, drawCommandBuffer[i], drawCommandBufferMemory[i], PersitantMapping, &drawCommandBufferMapped[i]);
        }
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createDescriptorPool(VkDescriptorPool& descriptorPool, uint32_t poolSize, int numTextures, uint32_t numBuffers, uint32_t numStorageBuffers, uint32_t numStorageIamges, bool hasDrawCommandBuffer, bool isDefferedShader)
    {
        std::vector<VkDescriptorPoolSize> poolSizes(numTextures+ numBuffers + numStorageBuffers + numStorageIamges + (int)hasDrawCommandBuffer * 2 + (int)isDefferedShader * 2);
        uint32_t bindings = 0;
        for (size_t i = 0; i < numBuffers; i++)
        {
            poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            bindings++;
        }
        for (size_t i = 0; i < numTextures; i++)
        {
            poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            bindings++;
        }
        for (size_t i = 0; i < numStorageBuffers; i++)
        {
            poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            bindings++;
        }
        for (size_t i = 0; i < numStorageIamges; i++)
        {
            poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
            bindings++;
        }
        if (hasDrawCommandBuffer) {
            for (size_t i = 0; i < 2; i++)
            {
                poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                bindings++;
            }
        }
        if (isDefferedShader) {
            for (size_t i = 0; i < 2; i++)
            {
                poolSizes[bindings].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                poolSizes[bindings].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
                bindings++;
            }
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * poolSize;

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets(std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& descriptorSets, VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& descriptorSetLayout, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers,
        UniformBufferID* storageUniforms, bool* storageUniformsLastFrame, uint32_t numStorageUniforms, VertexBufferID* storageVertex, bool* storageVertexLastFrame, uint32_t numStorageVertex, IndexBufferID* storageIndex, bool* storageIndexLastFrame,
        uint32_t numStorageIndex, TextureID* storageTextureIDs, bool* storageTextureLastFrame, uint32_t numstorageTextures, bool hasDrawCommandBuffer, bool isDefferedShader)
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }


        for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
            int lastFrame = j - 1;
            if (lastFrame < 0) lastFrame = MAX_FRAMES_IN_FLIGHT - 1;
            std::vector<VkDescriptorBufferInfo> BufferInfo(numBuffers);
            for (size_t i = 0; i < numBuffers; i++)
            {
                BufferInfo[i].buffer = uniformBuffers[bufferIDs[i]][j];
                BufferInfo[i].offset = 0;
                BufferInfo[i].range = uniformBuffersSize[bufferIDs[i]];
            }

            std::vector<VkDescriptorImageInfo> imageInfo(numTextureIDs);
            for (size_t i = 0; i < numTextureIDs; i++)
            {
                imageInfo[i].imageLayout = textureImageLayout[textureIDs[i]];
                imageInfo[i].imageView = textureImageViews[textureIDs[i]][j];
                imageInfo[i].sampler = imageSampler;
            }

            std::vector<VkDescriptorBufferInfo> storageUniformBufferInfo(numStorageUniforms);
            for (size_t i = 0; i < storageUniformBufferInfo.size(); i++)
            {
                storageUniformBufferInfo[i].buffer = uniformBuffers[storageUniforms[i]][storageUniformsLastFrame[i] ? lastFrame : j];
                storageUniformBufferInfo[i].offset = 0;
                storageUniformBufferInfo[i].range = uniformBuffersSize[storageUniforms[i]];
            }

            std::vector<VkDescriptorBufferInfo> storageVertexBufferInfo(numStorageVertex);
            for (size_t i = 0; i < storageVertexBufferInfo.size(); i++)
            {
                storageVertexBufferInfo[i].buffer = vertexBuffers[storageVertex[i]][storageVertexLastFrame[i] ? lastFrame : j];
                storageVertexBufferInfo[i].offset = 0;
                storageVertexBufferInfo[i].range = vertexBuffersSize[storageVertex[i]];
            }

            std::vector<VkDescriptorBufferInfo> storageIndexBufferInfo(numStorageVertex);
            for (size_t i = 0; i < storageIndexBufferInfo.size(); i++)
            {
                storageIndexBufferInfo[i].buffer = indexBuffers[storageIndex[i]][storageIndexLastFrame[i] ? lastFrame : j];
                storageIndexBufferInfo[i].offset = 0;
                storageIndexBufferInfo[i].range = indexBuffersSize[storageIndex[i]];
            }
            
            std::vector<VkDescriptorImageInfo> storageImageInfo(numstorageTextures);
            for (size_t i = 0; i < storageImageInfo.size(); i++)
            {
                storageImageInfo[i].imageLayout = textureImageLayout[storageTextureIDs[i]];
                storageImageInfo[i].imageView = textureImageViews[storageTextureIDs[i]][storageTextureLastFrame[i] ? lastFrame : j];
                storageImageInfo[i].sampler = imageSampler;
            }

            std::array<VkDescriptorBufferInfo,2> drawBufferInfo;

            if (hasDrawCommandBuffer) {
                drawBufferInfo[0].buffer = drawCommandBuffer[j];
                drawBufferInfo[0].offset = 0;
                drawBufferInfo[0].range = MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
                drawBufferInfo[1].buffer = drawCommandBuffer[lastFrame];
                drawBufferInfo[1].offset = 0;
                drawBufferInfo[1].range = MAX_COMMANDS * sizeof(VkDrawIndexedIndirectCommand);
            }

            std::vector<VkDescriptorImageInfo> defferedImageInfo(colorImageViewMsaaOut.size() + 1);
            if (isDefferedShader) {
                for (size_t i = 0; i < colorImageViewMsaaOut.size(); i++)
                {
                    defferedImageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    defferedImageInfo[i].imageView = colorImageViewMsaaOut[i];
                    defferedImageInfo[i].sampler = imageSampler;
                }
                defferedImageInfo[colorImageViewMsaaOut.size()].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                defferedImageInfo[colorImageViewMsaaOut.size()].imageView = depthImageView;
                defferedImageInfo[colorImageViewMsaaOut.size()].sampler = imageSampler;
            }

            std::vector<VkWriteDescriptorSet> descriptorWrites(numBuffers +  numTextureIDs + numStorageUniforms + numStorageIndex + numstorageTextures + numStorageVertex + (int)hasDrawCommandBuffer * 2 + (int)isDefferedShader * 2);

            int dstBinding = 0;

            for (size_t i = 0; i < numBuffers; i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pBufferInfo = &BufferInfo[i];
                dstBinding++;
            }

            for (size_t i = 0; i < numTextureIDs; i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pImageInfo = &imageInfo[i];
                dstBinding++;
            }

            for (size_t i = 0; i < storageUniformBufferInfo.size(); i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pBufferInfo = &storageUniformBufferInfo[i];
                dstBinding++;
            }

            for (size_t i = 0; i < storageVertexBufferInfo.size(); i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pBufferInfo = &storageVertexBufferInfo[i];
                dstBinding++;
            }

            for (size_t i = 0; i < storageIndexBufferInfo.size(); i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pBufferInfo = &storageIndexBufferInfo[i];
                dstBinding++;
            }

            for (size_t i = 0; i < storageImageInfo.size(); i++)
            {
                descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                descriptorWrites[dstBinding].dstBinding = dstBinding;
                descriptorWrites[dstBinding].dstArrayElement = 0;
                descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                descriptorWrites[dstBinding].descriptorCount = 1;
                descriptorWrites[dstBinding].pImageInfo = &storageImageInfo[i];
                dstBinding++;
            }

            if (hasDrawCommandBuffer) {
                for (size_t i = 0; i < drawBufferInfo.size(); i++)
                {
                    descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                    descriptorWrites[dstBinding].dstBinding = dstBinding;
                    descriptorWrites[dstBinding].dstArrayElement = 0;
                    descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descriptorWrites[dstBinding].descriptorCount = 1;
                    descriptorWrites[dstBinding].pBufferInfo = &drawBufferInfo[i];
                    dstBinding++;
                }
            }

            if (isDefferedShader) {
                for (size_t i = 0; i < defferedImageInfo.size(); i++)
                {
                    descriptorWrites[dstBinding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrites[dstBinding].dstSet = descriptorSets[j];
                    descriptorWrites[dstBinding].dstBinding = dstBinding;
                    descriptorWrites[dstBinding].dstArrayElement = 0;
                    descriptorWrites[dstBinding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrites[dstBinding].descriptorCount = 1;
                    descriptorWrites[dstBinding].pImageInfo = &defferedImageInfo[i];
                    dstBinding++;
                }
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }


    void createGPUSideOnlyBuffer(void* data, uint64_t dataSize, VkDeviceSize bufferSize, VkBuffer& buffer, VmaAllocation& bufferMemory, VkBufferUsageFlags useageFlags) {
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, Mapping);

        void* transferData;
        vmaMapMemory(allocator, stagingBufferMemory, &transferData);
        memcpy(transferData, data, (size_t)dataSize);
        vmaUnmapMemory(allocator, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | useageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory, NoMapping);

        copyBuffer(stagingBuffer, buffer, bufferSize);

        vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferMemory);
    }

    void createDefferdGraphicsPipline(std::string fragShaderPath)
    {
        auto vertShaderCode = readFile( rendererDir + "shaders/defferedVert.spv");
        auto fragShaderCode = readFile(fragShaderPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &defferedDescriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &defferedPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = defferedPipelineLayout;
        pipelineInfo.renderPass = defferedRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &defferedGraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }


    void createGraphicsPipline(std::string vertShaderPath, std::string fragShaderPath, VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline, VkDescriptorSetLayout& descriptorSetLayout, VertexValueType* vertexValues, uint32_t numVertexValues, VertexValueType* InstanceTypes, uint32_t numInstanceTypes,
        std::string tessContorlShaderPath, std::string tessEvaluationShaderPath, CullMode cullMode)
    {
        auto vertShaderCode = readFile(vertShaderPath);
        auto fragShaderCode = readFile(fragShaderPath);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";


        VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
        VkPipelineShaderStageCreateInfo tessEvalShaderStageInfo{};
        VkPipelineShaderStageCreateInfo tessConShaderStageInfo{};
        VkShaderModule tessConShaderModule;
        VkShaderModule tessEvalShaderModule;
        if (tessContorlShaderPath != "") {
            auto tessControlShaderCode = readFile(tessContorlShaderPath);
            auto tessEvalShaderCode = readFile(tessEvaluationShaderPath);

            tessConShaderModule = createShaderModule(tessControlShaderCode);
            tessEvalShaderModule = createShaderModule(tessEvalShaderCode);

            tessConShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            tessConShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            tessConShaderStageInfo.module = tessConShaderModule;
            tessConShaderStageInfo.pName = "main";

            tessEvalShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            tessEvalShaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            tessEvalShaderStageInfo.module = tessEvalShaderModule;
            tessEvalShaderStageInfo.pName = "main";


            pipelineTessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
            pipelineTessellationStateCreateInfo.pNext = nullptr;
            pipelineTessellationStateCreateInfo.flags = 0;
            pipelineTessellationStateCreateInfo.patchControlPoints = 4;
        }

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo,  tessConShaderStageInfo, tessEvalShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        uint32_t offset = 0;
        auto vertexBindingDescription = getBindingDescription(vertexValues, numVertexValues, VK_VERTEX_INPUT_RATE_VERTEX, 0);
        auto vertexAttributeDescriptions = getAttributeDescriptions(vertexValues, numVertexValues, 0, 0);
        auto instanceBindingDescription = getBindingDescription(InstanceTypes, numInstanceTypes, VK_VERTEX_INPUT_RATE_INSTANCE, 1);
        auto instanceAttributeDescriptions = getAttributeDescriptions(InstanceTypes, numInstanceTypes, 1, vertexAttributeDescriptions.size());

        std::array<VkVertexInputBindingDescription,2> bindingDescription = { vertexBindingDescription , instanceBindingDescription };
        vertexAttributeDescriptions.insert(vertexAttributeDescriptions.end(), instanceAttributeDescriptions.begin(), instanceAttributeDescriptions.end());

        vertexInputInfo.vertexBindingDescriptionCount = numInstanceTypes == 0 ? 1: 2;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = tessContorlShaderPath == "" ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = msaaSamples;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2 + (tessContorlShaderPath == "" ? 0: 2);
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        if(tessContorlShaderPath != "") pipelineInfo.pTessellationState = &pipelineTessellationStateCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        if (tessEvaluationShaderPath != "")
        {
            vkDestroyShaderModule(device, tessConShaderModule, nullptr);
            vkDestroyShaderModule(device, tessEvalShaderModule, nullptr);
        }
    }

    void createDefferedDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, int numTextures, uint32_t numBuffers)
    {
        uint32_t binding = 0;

        std::vector<VkDescriptorSetLayoutBinding> bindings(numTextures + numBuffers + 2);

        for (size_t i = 0; i < numBuffers; i++)
        {
            bindings[binding].binding = binding;
            bindings[binding].descriptorCount = 1;
            bindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[binding].pImmutableSamplers = nullptr;
            bindings[binding].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding++;
        }

        for (size_t i = 0; i < numTextures; i++)
        {
            bindings[binding].binding = binding;
            bindings[binding].descriptorCount = 1;
            bindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[binding].pImmutableSamplers = nullptr;
            bindings[binding].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding++;
        }

        for (size_t i = 0; i < 2; i++)
        {
            bindings[binding].binding = binding;
            bindings[binding].descriptorCount = 1;
            bindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[binding].pImmutableSamplers = nullptr;
            bindings[binding].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding++;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, ShaderStages* textureAccess, int numTextures, ShaderStages* bufferAccess, uint32_t numBuffers)
    {
        uint32_t binding = 0;

        std::vector<VkDescriptorSetLayoutBinding> bindings(numTextures + numBuffers);

        for (size_t i = 0; i < numBuffers; i++)
        {
            bindings[binding].binding = binding;
            bindings[binding].descriptorCount = 1;
            bindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindings[binding].pImmutableSamplers = nullptr;
            bindings[binding].stageFlags = bufferAccess[i];
            binding++;
        }

        for (size_t i = 0; i < numTextures; i++)
        {
            bindings[binding].binding = binding;
            bindings[binding].descriptorCount = 1;
            bindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[binding].pImmutableSamplers = nullptr;
            bindings[binding].stageFlags = textureAccess[i];
            binding++;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }


    void createComputeDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, uint32_t numUbos, uint32_t numImages, uint32_t numStorageBuffer, uint32_t numStorageImages, bool hasDrawCommandBuffer) {
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(numImages + numStorageBuffer + numStorageImages + numUbos + (int)hasDrawCommandBuffer * 2);
        uint32_t binding = 0;

        for (size_t i = 0; i < numUbos; i++)
        {
            layoutBindings[binding].binding = binding;
            layoutBindings[binding].descriptorCount = 1;
            layoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBindings[binding].pImmutableSamplers = nullptr;
            layoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            binding++;
        }

        for (size_t i = 0; i < numImages; i++)
        {
            layoutBindings[binding].binding = binding;
            layoutBindings[binding].descriptorCount = 1;
            layoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBindings[binding].pImmutableSamplers = nullptr;
            layoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            binding++;
        }

        for (size_t i = 0; i < numStorageBuffer; i++)
        {
            layoutBindings[binding].binding = binding;
            layoutBindings[binding].descriptorCount = 1;
            layoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            layoutBindings[binding].pImmutableSamplers = nullptr;
            layoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            binding++;
        }

        for (size_t i = 0; i < numStorageImages; i++)
        {
            layoutBindings[binding].binding = binding;
            layoutBindings[binding].descriptorCount = 1;
            layoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            layoutBindings[binding].pImmutableSamplers = nullptr;
            layoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            binding++;
        }

        if (hasDrawCommandBuffer)
        {
            for (size_t i = 0; i < 2; i++)
            {
                layoutBindings[binding].binding = binding;
                layoutBindings[binding].descriptorCount = 1;
                layoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBindings[binding].pImmutableSamplers = nullptr;
                layoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
                binding++;
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = binding;
        layoutInfo.pBindings = layoutBindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute descriptor set layout!");
        }
    }


    void createComputePipeline(std::string computeShaderFilePath, VkDescriptorSetLayout& descriptorSetLayout, VkPipelineLayout& pipleineLayout, VkPipeline& pipeline) {
        auto computeShaderCode = readFile(computeShaderFilePath);

        VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShaderModule;
        computeShaderStageInfo.pName = "main";

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipleineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipleineLayout;
        pipelineInfo.stage = computeShaderStageInfo;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        vkDestroyShaderModule(device, computeShaderModule, nullptr);
    }

    VkCommandBuffer beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }


    ComputeID getNewComputeID(uint16_t computePass) {
        if (unfilledComputeIDs.size() <= computePass) { 
            int temp = unfilledComputeIDs.size();
            unfilledComputeIDs.resize(computePass + 1);
            computes.resize(computePass + 1);
            computestageEvents.resize(computePass + 1);
            for (int i = temp; i < computePass + 1; i++) {
                VkEventCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
                if (vkCreateEvent(device, &createInfo, nullptr, &computestageEvents[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create new stage compute stage!");
                }
            }
        }
        if (!unfilledComputeIDs[computePass].empty()) {
            ComputeID i = unfilledComputeIDs[computePass].back();
            unfilledComputeIDs.pop_back();
            return i;
        }
        ComputeID i = (ComputeID)computes[computePass].size();
        computes[computePass].resize(i + 1);
        return (ComputeID)((i << 16) + computePass);
    }

    RenderObjectID getNewRenderObjectID() {
        if (!unfilledRenderObjectIDs.empty()) {
            RenderObjectID i = unfilledRenderObjectIDs.back();
            unfilledRenderObjectIDs.pop_back();
            return i;
        }
        RenderObjectID i = (RenderObjectID)renderObjects.size();
        renderObjects.resize(i + 1);
        return i;
    }

    MaterialID getNewMaterialID() {
        if (!unfilledMaterialIDs.empty()) {
            MaterialID i = unfilledMaterialIDs.back();
            unfilledMaterialIDs.pop_back();
            return i;
        }
        MaterialID i = (MaterialID)materialShaderIDs.size();
        materialShaderIDs.resize(i + 1);
        materialDescriptorSets.resize(i + 1);
        return i;
    }

    ShaderID getNewShaderID() {
        if (!unfilledShaderIDs.empty()) {
            ShaderID i = unfilledShaderIDs.back();
            unfilledShaderIDs.pop_back();
            return i;
        }
        ShaderID i = (ShaderID)shaderGraphicsPipelines.size();
        shaderGraphicsPipelines.resize(i + 1);
        shaderDescriptorSetLayouts.resize(i + 1);
        shaderPipelineLayouts.resize(i + 1);
        shaderDescriptorPools.resize(i + 1);
        return i;
    }

    VertexBufferID getNewVertexBufferID() {
        if (!unfilledVertexBufferIDs.empty()) {
            VertexBufferID i = unfilledVertexBufferIDs.back();
            unfilledVertexBufferIDs.pop_back();
            return i;
        }
        VertexBufferID i = (VertexBufferID)vertexBuffers.size();
        vertexBuffers.resize(i + 1);
        vertexBufferMemorys.resize(i + 1);
        vertexBuffersMapped.resize(i + 1);
        vertexBufferData.resize(i + 1);
        vertexBuffersSize.resize(i + 1);
        vertexUpToDate.resize(i + 1);
        vertexNumInstances.resize(i + 1);
        return i;
    }

    UniformBufferID getNewUniformBufferID() {
        if (!unfilledUniformBufferIDs.empty()) {
            UniformBufferID i = unfilledUniformBufferIDs.back();
            unfilledUniformBufferIDs.pop_back();
            return i;
        }
        UniformBufferID i = (UniformBufferID)uniformBuffers.size();
        uniformBuffers.resize(i + 1);
        uniformBuffersMemory.resize(i + 1);
        uniformBuffersMapped.resize(i + 1);
        uniformBuffersSize.resize(i + 1);
        uniformBufferData.resize(i + 1);
        uniformUpToDate.resize(i + 1);
        return i;
    }

    IndexBufferID getNewIndexBufferID() {
        if (!unfilledIndexBufferIDs.empty()) {
            IndexBufferID i = unfilledIndexBufferIDs.back();
            unfilledIndexBufferIDs.pop_back();
            return i;
        }
        IndexBufferID i = (IndexBufferID)indexBuffers.size();
        indexBuffers.resize(i + 1);
        indexBufferMemorys.resize(i + 1);
        indexBuffersMapped.resize(i + 1);
        indexBufferData.resize(i + 1);
        indexNumIndices.resize(i + 1);
        indexUpToDate.resize(i + 1);
        indexBuffersSize.resize(i + 1);
        return i;
    }

    ComputeShaderID getNewComputeShaderID() {
        if (!unfilledComputeShaderIDs.empty()) {
            ComputeShaderID i = unfilledComputeShaderIDs.back();
            unfilledComputeShaderIDs.pop_back();
            return i;
        }
        ComputeShaderID i = (ComputeShaderID)computePipeline.size();
        computeDescriptorSetLayout.resize(i + 1);
        computePipelineLayout.resize(i + 1);
        computePipeline.resize(i + 1);
        computeDescriptorPool.resize(i + 1);
        return i;
    }


    TextureID getNewTextrueID() {
        if (!unfilledTextureIDs.empty()) {
            TextureID i = unfilledTextureIDs.back();
            unfilledTextureIDs.pop_back();
            return i;
        }
        TextureID i = (TextureID)textureImages.size();
        textureImages.resize(i + 1);
        textureImageMemorys.resize(i + 1);
        textureImageViews.resize(i + 1);
        textureImageLayout.resize(i + 1);
        return i;
    }

    void cleanupSwapChain() {
        vmaDestroyImage(allocator, depthImage, depthImageMemory);
        vkDestroyImageView(device, depthImageView, nullptr);

        for (size_t i = 0; i < colorImage.size(); i++)
        {
            vmaDestroyImage(allocator, colorImage[i], colorImageMemory[i]);
            vkDestroyImageView(device, colorImageView[i], nullptr);

            vmaDestroyImage(allocator, colorImageMsaaOut[i], colorImageMemoryMsaaOut[i]);
            vkDestroyImageView(device, colorImageViewMsaaOut[i], nullptr);
        }

        for (auto framebuffer : swapChainFramebuffers[0]) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        for (auto framebuffer : swapChainFramebuffers[1]) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics synchronization objects for a frame!");
            }
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create compute synchronization objects for a frame!");
            }
        }
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void createCommandBuffers() {

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void createFramebuffers() {
        swapChainFramebuffers[0].resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::vector<VkImageView> attachments(colorImageView.size() * 2 + 1);
            int attachIndex = 0;
            for (size_t i = 0; i < colorImageView.size(); i++)
            {
                attachments[attachIndex] = colorImageView[i];
                attachIndex++;
            }
            attachments[attachIndex] = depthImageView;
            attachIndex++;
            for (size_t i = 0; i < colorImageViewMsaaOut.size(); i++)
            {
                attachments[attachIndex] = colorImageViewMsaaOut[i];
                attachIndex++;
            }

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[0][i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }

        swapChainFramebuffers[1].resize(swapChainImageViews.size());

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 1> attachments = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = defferedRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[1][i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createComputeCommandBuffers() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = computeCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate compute command buffers!");
        }
    }


    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics command pool!");
        }
        if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.computeFamily) {
            poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
            if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics command pool!");
            }
        }
        else {
            computeCommandPool = commandPool;
        }
    }

    void createColorResources(int numColorResorces) {
        colorImage.resize(numColorResorces);
        colorImageView.resize(numColorResorces);
        colorImageMemory.resize(numColorResorces);
        for (size_t i = 0; i < numColorResorces; i++)
        {
            VkFormat colorFormat = swapChainImageFormat;
            createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage[i], colorImageMemory[i]);
            colorImageView[i] = createImageView(colorImage[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, false);
        }
    }

    void createColorMsaaOutResources(int numColorResorces) {
        colorImageMsaaOut.resize(numColorResorces);
        colorImageViewMsaaOut.resize(numColorResorces);
        colorImageMemoryMsaaOut.resize(numColorResorces);
        for (size_t i = 0; i < numColorResorces; i++)
        {
            VkFormat colorFormat = swapChainImageFormat;
            createImage(swapChainExtent.width, swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImageMsaaOut[i], colorImageMemoryMsaaOut[i]);
            colorImageViewMsaaOut[i] = createImageView(colorImageMsaaOut[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, false);
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1, false);
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VmaAllocation& bufferMemory, BufferMappingType mapping, VmaAllocationInfo* allocationInfo) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        switch (mapping)
        {
        case NoMapping:
            break;
        case Mapping:
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        case PersitantMapping:
            allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        default:
            break;
        }

        if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory, allocationInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }
    }

    std::array<uint32_t,3> getOffsetVertexValue(VertexValueType vertexValue) {
        switch (vertexValue) {
        case VTfloat:
            return { 4, 1, VK_FORMAT_R32_SFLOAT };
        case VTfloat2:
            return { 8, 1, VK_FORMAT_R32G32_SFLOAT };
        case VTfloat3:
            return { 12, 1, VK_FORMAT_R32G32B32_SFLOAT };
        case VTfloat4:
            return { 16, 1, VK_FORMAT_R32G32B32A32_SFLOAT };
        case VTfloat4x4:
            return { 16, 4, VK_FORMAT_R32G32B32A32_SFLOAT };
        case VTfloat3x4:
            return { 12, 4, VK_FORMAT_R32G32B32_SFLOAT };
        case VTfloat2x4:
            return { 8, 4, VK_FORMAT_R32G32_SFLOAT };
        case VTfloat4x3:
            return { 16, 3, VK_FORMAT_R32G32B32A32_SFLOAT };
        case VTfloat3x3:
            return { 12, 3, VK_FORMAT_R32G32B32_SFLOAT };
        case VTfloat2x3:
            return { 8, 3, VK_FORMAT_R32G32_SFLOAT };
        case VTfloat4x2:
            return { 16, 2, VK_FORMAT_R32G32B32A32_SFLOAT };
        case VTfloat3x2:
            return { 12, 2, VK_FORMAT_R32G32B32_SFLOAT };
        case VTfloat2x2:
            return { 8, 2, VK_FORMAT_R32G32_SFLOAT };
        default:
            throw std::runtime_error("invalid vertex values when creating shader!");
            break;
        }
        return {0,0,0};
    }

    VkVertexInputBindingDescription getBindingDescription(VertexValueType* VertexValues, uint32_t numVertexValues, VkVertexInputRate inputRate, uint32_t binding) {

        uint32_t vertexSize = 0;
        for (size_t i = 0; i < numVertexValues; i++)
        {
            std::array<uint32_t,3> offsetValues = getOffsetVertexValue(VertexValues[i]);
            vertexSize += offsetValues[0] * offsetValues[1];
        }
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = binding;
        bindingDescription.stride = vertexSize;
        bindingDescription.inputRate = inputRate;

        return bindingDescription;
    }

    std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(VertexValueType* VertexValues, uint32_t numVertexValues, uint32_t binding, uint32_t layoutOffset) {
        uint32_t offset = 0;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        for (size_t i = 0; i < numVertexValues; i++)
        {
            std::array<uint32_t, 3> offsetValues = getOffsetVertexValue(VertexValues[i]);
            for (size_t i = 0; i < offsetValues[1]; i++)
            {
                size_t j = attributeDescriptions.size();
                attributeDescriptions.resize(j+1);
                attributeDescriptions[j].binding = binding;
                attributeDescriptions[j].location = j + layoutOffset;
                attributeDescriptions[j].format = (VkFormat)offsetValues[2];
                attributeDescriptions[j].offset = offset;
                offset += offsetValues[0];
            }
        }

        return attributeDescriptions;
    }

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VmaAllocation& imageMemory, uint32_t arrayLayers, bool cube) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = cube ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = arrayLayers;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = numSamples;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

        if (vmaCreateImage(allocator, &imageInfo, &allocInfo, &image, &imageMemory, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }
    }

    void createDefferedRenderPass() {
        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 0;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentResolveRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachmentResolve;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &defferedRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create deffered render pass!");
        }
    }

    void createRenderPass(int numColorAttachments) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        int attachment = 0;
        std::vector<VkAttachmentReference> colorAttachmentRef(numColorAttachments);
        for (size_t i = 0; i < numColorAttachments; i++)
        {
            colorAttachmentRef[i].attachment = attachment;
            colorAttachmentRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment++;
        }

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = attachment;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachment++;

        std::vector<VkAttachmentReference> colorAttachmentResolveRef(numColorAttachments);
        for (size_t i = 0; i < numColorAttachments; i++)
        {
            colorAttachmentResolveRef[i].attachment = attachment;
            colorAttachmentResolveRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment++;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = numColorAttachments;
        subpass.pColorAttachments = colorAttachmentRef.data();
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = colorAttachmentResolveRef.data();

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }


    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, false);
        }
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, bool cube) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = cube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = cube ? 6: 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageView;
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.tessellationShader = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        int64_t bestRating = -9223372036854775808;
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                int64_t rating = ratePhysicalDevice(device);
                if (rating > bestRating) {
                    physicalDevice = device;
                    msaaSamples = getMaxUsableSampleCount();
                    bestRating = rating;
                }
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    int64_t ratePhysicalDevice(VkPhysicalDevice phDevice) {
        int rating = 0;
        auto props = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(phDevice, &props);
        if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            rating += 40000000;
        }
        else if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            rating -= 40000000;
        }

        VkPhysicalDeviceMemoryProperties memoryProps = {};
        vkGetPhysicalDeviceMemoryProperties(phDevice, &memoryProps);

        auto heapsPointer = memoryProps.memoryHeaps;
        auto heaps = std::vector<VkMemoryHeap>(heapsPointer, heapsPointer + memoryProps.memoryHeapCount);

        for (const auto& heap : heaps)
        {
            if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                rating += heap.size;
            }
        }
        return rating;
    }

    VkSampleCountFlagBits getMaxUsableSampleCount() {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && supportedFeatures.tessellationShader;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT && (!indices.presentFamily || indices.presentFamily == indices.graphicsFamily)) {
                indices.computeFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport && (!indices.presentFamily || indices.presentFamily == indices.graphicsFamily)) {
                indices.presentFamily = i;
            }

            i++;
        }

        return indices;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }


    void createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

}


#endif // VULKANRENDERER
