#include <GenericRenderer/DefaultRenderer.h>

namespace MZ {
	std::string rendererDir;


	RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, BoundingSphere boundingSphere) {
		RenderObjectID i = addRenderObject(material, vertexBuffer, indexBuffer);
		addToCullingBuffer(boundingSphere, i, 1);
		return i;
	}
	RenderObjectID addRenderObject(MaterialID material, VertexBufferID vertexBuffer, IndexBufferID indexBuffer, VertexBufferID instanceBuffer, BoundingSphere boundingSphere) {
		RenderObjectID i = addRenderObject(material, vertexBuffer, indexBuffer, instanceBuffer);
		addToCullingBuffer(boundingSphere, i, getInstanceCount(instanceBuffer));
		return i;
	}

}
