#include <GenericRenderer/defaultPrivate.h>
#include <GenericRenderer/DefaultRenderer.h>
#include <Mesmerize/defaults.h>
#include <Mesmerize/Renderer.h>



namespace MZ {

	UniformBufferID mainCameraBuffer;
	UniformBufferID mainCameraFullcrumBuffer;
	UniformBufferID cullingBuffer;
	ComputeShaderID cullingShader;
	ComputeID mainCullingCompute;
	extern void setupNoDefaults(GLFWwindow* window, int numGBuffers, std::string pathToRendererDir);

	void setup(GLFWwindow* window, int numGBuffers, std::string pathToRendererDir){
		setupNoDefaults(window, numGBuffers, pathToRendererDir);
		setupDefaults();
	}

	void drawFrame() {
		glm::mat4* camBuf = (glm::mat4*)getCPUMutUniformBufferData(mainCameraBuffer);
		updateCameraFullcrumBuffer(camBuf[1] * camBuf[0], mainCameraFullcrumBuffer);
		renderFrame();
	}

	void setupDefaults() {
		createMainCameraBuffer();
		createCullingBuffer();
		cullingShader = createComputeShader( rendererDir + "/shaders/culling.spv", 1, 2, 0, 0, 0, true);
		mainCullingCompute = createCullingCompute(mainCameraBuffer, mainCameraFullcrumBuffer);
	}

	void createMainCameraBuffer() {
		glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), MZ::getRenderWidth() / (float)MZ::getRenderHeight(), 0.1f, 10.0f);
		proj[1][1] *= -1;
		glm::mat4 buf[] = { view, proj };
		mainCameraBuffer = createCPUMutUniformBuffer(&buf, sizeof(glm::mat4) * 2, sizeof(glm::mat4) * 2);

		cameraFullcrums fullcrums = {};
		mainCameraFullcrumBuffer = createCPUMutUniformBuffer(&fullcrums, sizeof(cameraFullcrums), sizeof(cameraFullcrums));
	}

	void updateCameraFullcrumBuffer(glm::mat4 viewproj, UniformBufferID buffer) {
		cameraFullcrums fullcrums;
		fullcrums.left = glm::vec4(viewproj[3][0] + viewproj[0][0], viewproj[3][1] + viewproj[0][1], viewproj[3][2] + viewproj[0][2], viewproj[3][3] + viewproj[0][3]);
		fullcrums.right = glm::vec4(viewproj[3][0] - viewproj[0][0], viewproj[3][1] - viewproj[0][1], viewproj[3][2] - viewproj[0][2], viewproj[3][3] - viewproj[0][3]);
		fullcrums.bottom = glm::vec4(viewproj[3][0] + viewproj[1][0], viewproj[3][1] + viewproj[1][1], viewproj[3][2] + viewproj[1][2], viewproj[3][3] + viewproj[1][3]);
		fullcrums.top = glm::vec4(viewproj[3][0] - viewproj[1][0], viewproj[3][1] - viewproj[1][1], viewproj[3][2] - viewproj[1][2], viewproj[3][3] - viewproj[1][3]);
		fullcrums.near = glm::vec4(viewproj[3][0] + viewproj[2][0], viewproj[3][1] + viewproj[2][1], viewproj[3][2] + viewproj[2][2], viewproj[3][3] + viewproj[2][3]);
		fullcrums.far = glm::vec4(viewproj[3][0] - viewproj[2][0], viewproj[3][1] - viewproj[2][1], viewproj[3][2] - viewproj[2][2], viewproj[3][3] - viewproj[2][3]);
		updateCPUMutUniformBuffer(buffer, &fullcrums, sizeof(cameraFullcrums));
	}
	
	void createCullingBuffer() {
		glm::vec4 defaultSphere(0,0,0,-1);
		std::vector<glm::vec4> defaultSpheres(MAX_COMMANDS);
		std::fill(defaultSpheres.begin(), defaultSpheres.end(), defaultSphere);
		std::vector<uint32_t> defaultInstancCount(MAX_COMMANDS);
		std::fill(defaultInstancCount.begin(), defaultInstancCount.end(), 0);
		void* bufferDefault = malloc((sizeof(glm::vec4) + sizeof(uint32_t)) * MAX_COMMANDS);
		memcpy(bufferDefault, defaultSpheres.data(), defaultSpheres.size() * sizeof(glm::vec4));
		memcpy((void*)((intptr_t)bufferDefault + defaultSpheres.size() * sizeof(glm::vec4)), defaultInstancCount.data(), defaultInstancCount.size() * sizeof(uint32_t));
		cullingBuffer = createCPUMutUniformBuffer(bufferDefault, (sizeof(glm::vec4) + sizeof(uint32_t)) * MAX_COMMANDS, (sizeof(glm::vec4) + sizeof(uint32_t)) * MAX_COMMANDS);
		free(bufferDefault);
	}

	ComputeID createCullingCompute(UniformBufferID cameraBuffer, UniformBufferID cameraFullcrumBuffer) {
		std::array<UniformBufferID, 3> cullingUniformBuffers = { cullingBuffer, cameraFullcrumBuffer };
		return addCompute(cullingShader, MAX_COMMANDS/32, 1, 1, 0, cullingUniformBuffers.data(), 2, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, nullptr, nullptr, 0, true);
	}

	void addToCullingBuffer(BoundingSphere& boundingSphere, RenderObjectID renderObjectID, uint32_t instanceCount) {
		updateCPUMutUniformBuffer(cullingBuffer, &boundingSphere, sizeof(glm::vec4), renderObjectID * sizeof(glm::vec4));
		updateCPUMutUniformBuffer(cullingBuffer, &instanceCount, sizeof(uint32_t), MAX_COMMANDS * sizeof(glm::vec4) + renderObjectID * sizeof(uint32_t));
	}

}