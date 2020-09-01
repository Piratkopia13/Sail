#pragma once
#include "Sail/api/VertexBuffer.h"
#include "VkAPI.h"

class VkVertexBuffer : public VertexBuffer {
public:
	VkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates = false);
	~VkVertexBuffer();

	virtual void bind(void* cmdList) override;
	void update(Mesh::Data& data);

	// TODO: make these less methods into better ones that doesn't require renderers to call reset
	void setAsUpdated();
	bool hasBeenUpdated() const;
	void resetHasBeenUpdated();

private:
	VkAPI* m_context;

};