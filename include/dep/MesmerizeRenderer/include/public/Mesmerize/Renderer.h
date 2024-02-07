#pragma once
namespace MZ{
#ifdef VULKANRENDERER
    #define GraphicsAPI "Vulkan"
#else
    #define GraphicsAPI "No Valid API"

#endif // VULKANRENDERER

#define MAX_COMMANDS 1024

    BOOST_STRONG_TYPEDEF(uint16_t, ShaderID);
    BOOST_STRONG_TYPEDEF(uint16_t, RenderObjectID);
    BOOST_STRONG_TYPEDEF(uint16_t, TextureID);
    BOOST_STRONG_TYPEDEF(uint16_t, MaterialID);
    BOOST_STRONG_TYPEDEF(uint16_t, UniformBufferID);
    BOOST_STRONG_TYPEDEF(uint16_t, VertexBufferID);
    BOOST_STRONG_TYPEDEF(uint16_t, IndexBufferID);
    BOOST_STRONG_TYPEDEF(uint16_t, ComputeShaderID);
    BOOST_STRONG_TYPEDEF(uint32_t, ComputeID);

    // negitive radius means no culling
    struct BoundingSphere {
        BoundingSphere(glm::vec3 positon, float radius) {
            sphereInfo = glm::vec4(positon,radius);
        }
        glm::vec4 sphereInfo;
    };

    enum VertexValueType {
        VTfloat4,
        VTfloat3,
        VTfloat2,
        VTfloat,
        VTfloat4x4,
        VTfloat4x3,
        VTfloat4x2,
        VTfloat3x4,
        VTfloat3x3,
        VTfloat3x2,
        VTfloat2x4,
        VTfloat2x3,
        VTfloat2x2,
    };

    typedef int ShaderStages;
    typedef enum ShaderStage {
        SSVert = VK_SHADER_STAGE_VERTEX_BIT,
        SSFrag = VK_SHADER_STAGE_FRAGMENT_BIT,
        SSTessCon = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        SSTessEval = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
    };


    /// <summary>
    /// must call before doing anything
    /// </summary>
    /// <param name="window"> surface to render to </param>
    void setup(GLFWwindow* window, int numGBuffers, std::string pathToRendererDir);


    /// <summary>
    /// call when you no longer want to render or at end of program
    /// </summary>
    void cleanup();

    /// <summary>
    /// call whenver a new frame needs to be drawn
    /// </summary>
    void drawFrame();

    /// <summary>
    /// gets the resultion of the rendered output image width
    /// </summary>
    int getRenderWidth();

    /// <summary>
    /// gets the resultion of the rendered output image height
    /// </summary>
    int getRenderHeight();

    /// <summary>
    /// must call before calling drawFrame. sets the deffered shader
    /// </summary>
    void setDefferedShader(std::string fragShader, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers);

    void setSkybox(TextureID cubemap);

    /// <summary>
    /// Call once then an object will be rendered every drawFrame
    /// </summary>
    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer);
    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, VertexBufferID instanceBuffer);
    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, BoundingSphere boundingSphere);
    RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, VertexBufferID instanceBuffer, BoundingSphere boundingSphere);


    /// <summary>
    /// Call once then this compute shader will be called every frame
    /// </summary>
    ComputeID addCompute(ComputeShaderID computeShader, uint32_t xDispatch, uint32_t yDispatch, uint32_t zDispatch, uint16_t computePass, UniformBufferID* UniformBuffers, uint32_t numUniformBuffers,
        TextureID* textures, uint32_t numTextues, UniformBufferID* storageUniforms, bool* storageUniformsLastFrame, uint32_t numStorageUniforms, VertexBufferID* storageVertex, bool* storageVertexLastFrame,
        uint32_t numStorageVertex, IndexBufferID* storageIndex, bool* storageIndexLastFrame, uint32_t numStorageIndex, TextureID* storageTexture, bool* storageTextureLastFrame, uint32_t numStorageTexture, bool hasDrawCommandBuffer);

    /// <summary>
    /// Material defines the shader as well as all binding related to it
    /// </summary>
    MaterialID createMaterial(ShaderID shaderID, TextureID* textureIDs, uint32_t numTextureIDs, UniformBufferID* bufferIDs, uint32_t numBuffers);



    enum CullMode {
        FrontCull = VK_CULL_MODE_FRONT_BIT,
        BackCull = VK_CULL_MODE_BACK_BIT,
        NoCull = VK_CULL_MODE_NONE,
        BothCull = VK_CULL_MODE_FRONT_AND_BACK,
    };
    /// <summary>
    /// a vert and fragment shader. multiple materials cand be created from a single shader
    /// </summary>
    ShaderID createShader(std::string vertShaderPath, std::string fragShaderPath, uint32_t maxNumberOfMaterial, ShaderStages* textureAccess, uint32_t numTextures, ShaderStages* bufferAccess, uint32_t numBuffers, VertexValueType* VertexValues, uint32_t numVertexValues,
        VertexValueType* InstanceTypes, uint32_t numInstanceTypes, CullMode cullMode);
    ShaderID createShader(std::string vertShaderPath, std::string fragShaderPath, std::string tessalizationControlShaderPath, std::string tessalizationEvaluationShaderPath, uint32_t maxNumberOfMaterial, ShaderStages* textureAccess, uint32_t numTextures,
        ShaderStages* bufferAccess, uint32_t numBuffers, VertexValueType* VertexValues, uint32_t numVertexValues, VertexValueType* InstanceTypes, uint32_t numInstanceTypes, CullMode cullMode);

    /// <summary>
    /// a compute shader this can be dispached in multiple way by adding a compute.
    /// </summary>
    ComputeShaderID createComputeShader(std::string computeShaderPath, uint32_t maxNumberOfComputes, uint32_t numUniformBuffers, uint32_t numStaticTextures, uint32_t numStorageBuffers,
        uint32_t numStorageTextues, bool hasDrawCommandBuffer = false);


    enum ImageFormat {
        IFSRGBA8 = VK_FORMAT_R8G8B8A8_SRGB,
        IFFloat4 = VK_FORMAT_R32G32B32A32_SFLOAT,
        IFFloat3 = VK_FORMAT_R32G32B32_SFLOAT,
        IFFloat2 = VK_FORMAT_R32G32_SFLOAT,
        IFFloat = VK_FORMAT_R32_SFLOAT,
    };
    /// <summary>
    /// loads a texture to be used by other parts of the renderer
    /// </summary>
    TextureID createConstTexture(std::string textureFilepath);
    TextureID createConstTexture(void* data ,uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap);
    TextureID createGPUMutTexture(uint32_t width, uint32_t height, ImageFormat imageFormat);
    TextureID createGPUMutTexture(void* data, uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap);
    TextureID createGPUMutTextureSingle(uint32_t width, uint32_t height, ImageFormat imageFormat);
    TextureID createGPUMutTextureSingle(void* data, uint32_t width, uint32_t height, ImageFormat imageFormat, bool cubemap);


    /// <summary>
    /// a vertex buffer can hold vertex data or instance data. you must specify the types of mutability by calling the right constructor
    /// </summary>
    VertexBufferID createConstVertexBuffer(void* vertices, uint32_t numVertices,uint64_t bufferSize);
    VertexBufferID createCPUMutVertexBuffer(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize);
    VertexBufferID createCPUMutVertexBufferSingle(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize);
    VertexBufferID createGPUMutVertexBuffer(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize);
    VertexBufferID createGPUMutVertexBufferSingle(void* vertices, uint32_t numVertices, uint32_t vertexSize, uint64_t bufferSize);


    /// <summary>
    /// a index buffer can hold indexs connecting vertices. you must specify the types of mutability by calling the right constructor
    /// </summary>
    IndexBufferID createConstIndexBuffer(void* indices, uint64_t bufferSize);
    IndexBufferID createCPUMutIndexBuffer(void* indices, uint32_t numIndices, uint64_t bufferSize);
    IndexBufferID createCPUMutIndexBufferSingle(void* indices, uint32_t numIndices, uint64_t bufferSize);
    IndexBufferID createGPUMutIndexBuffer(void* indices, uint32_t numIndices, uint64_t bufferSize);
    IndexBufferID createGPUMutIndexBufferSingle(void* indices, uint32_t numIndices, uint64_t bufferSize);


    /// <summary>
    /// a uniform buffer can be made to pass information in to shaders. you must specify the types of mutability by calling the right constructor
    /// </summary>
    UniformBufferID createConstUniformBuffer(void* data, uint32_t bufferSize);
    UniformBufferID createCPUMutUniformBuffer(void* data, uint32_t dataSize, uint32_t bufferSize);
    UniformBufferID createCPUMutUniformBufferSingle(void* data, uint32_t dataSize, uint32_t bufferSize);
    UniformBufferID createGPUMutUniformBuffer(void* data, uint32_t dataSize, uint32_t bufferSize);
    UniformBufferID createGPUMutUniformBufferSingle(void* data, uint32_t dataSize, uint32_t bufferSize);



    /// <summary>
    /// Deletes The Resource
    /// </summary>
    void deleteResourceCPU(UniformBufferID id);
    void deleteResourceCPUSingle(UniformBufferID id);
    void deleteResourceConst(UniformBufferID id);
    void deleteResourceGPU(UniformBufferID id);
    void deleteResourceGPUSinlge(UniformBufferID id);
    void deleteResourceGPU(VertexBufferID id);
    void deleteResourceCPU(VertexBufferID id);
    void deleteResourceGPUSingle(VertexBufferID id);
    void deleteResourceCPUSingle(VertexBufferID id);
    void deleteResourceConst(VertexBufferID id);
    void deleteResourceConst(IndexBufferID id);
    void deleteResourceCPU(IndexBufferID id);
    void deleteResourceGPU(IndexBufferID id);
    void deleteResourceCPUSingle(IndexBufferID id);
    void deleteResourceGPUSingle(IndexBufferID id);
    void deleteResource(RenderObjectID id);
    void deleteResource(ComputeID id);
    void deleteResource(ShaderID id);
    void deleteResource(ComputeShaderID id);
    void deleteResourceConst(TextureID id);
    void deleteResourceGPU(TextureID id);
    void deleteResourceGPUSingle(TextureID id);
    void deleteResource(MaterialID id);


    /// <summary>
    /// update a part or all of a buffer. must be CPUMutable buffer or will crash
    /// </summary>
    void updateCPUMutUniformBuffer(UniformBufferID buffer, void* data, uint32_t dataSize, uint32_t offset = 0);
    void updateCPUMutVertexBuffer(VertexBufferID buffer, void* data, uint32_t dataSize, uint32_t offset = 0);
    void updateCPUMutIndexBuffer(IndexBufferID buffer, void* data, uint32_t dataSize, uint32_t offset = 0);

    /// <summary>
    /// get a buffers data can only be done for cpu buffers
    /// </summary>
    void* getCPUMutUniformBufferData(UniformBufferID buffer);
    void* getCPUMutVertexBufferData(VertexBufferID buffer);
    void* getCPUMutIndexBufferData(IndexBufferID buffer);


    uint32_t getInstanceCount(VertexBufferID vertexBuffer);

}