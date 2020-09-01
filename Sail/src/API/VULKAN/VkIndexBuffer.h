#pragma once
#include "Sail/api/IndexBuffer.h"
#include "VkAPI.h"

class VkIndexBuffer : public IndexBuffer {
public:
	VkIndexBuffer(Mesh::Data& modelData);
	~VkIndexBuffer();

	virtual void bind(void* cmdList) override;

private:
	VkAPI* m_context;
};

