#pragma once
#include <Mesmerize/Renderer.h>
#include <pch.h>



namespace MZ {

	void createCullingBuffer();

	void createMainCameraBuffer();

	void addToCullingBuffer(BoundingSphere& boundingSphere, RenderObjectID renderObjectID, uint32_t instanceCount);

	void updateCameraFullcrumBuffer(glm::mat4 viewproj, UniformBufferID buffer);

	ComputeID createCullingCompute(UniformBufferID cameraBuffer, UniformBufferID cameraFullcrumBuffer);
}