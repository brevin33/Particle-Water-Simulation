#pragma once
#include <Mesmerize/Renderer.h>

namespace MZ {

	void setupDefaults();

	extern UniformBufferID mainCameraBuffer;
	extern UniformBufferID mainCameraFullcrumBuffer;
	struct cameraFullcrums {
		glm::vec4 left;
		glm::vec4 right;
		glm::vec4 top;
		glm::vec4 bottom;
		glm::vec4 near;
		glm::vec4 far;
	};
	extern UniformBufferID cullingBuffer;
	extern ComputeShaderID cullingShader;
	extern ComputeID mainCullingCompute;
}