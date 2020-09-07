#pragma once
#include "Sail/api/VertexBuffer.h"
#include "SVkAPI.h"

class SVkVertexBuffer : public VertexBuffer {
public:
	SVkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates = false);
	~SVkVertexBuffer();

	virtual void bind(void* cmdList) override;
	void update(Mesh::Data& data);

	// TODO: make these less methods into better ones that doesn't require renderers to call reset
	void setAsUpdated();
	bool hasBeenUpdated() const;
	void resetHasBeenUpdated();

private:
	SVkAPI* m_context;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

};