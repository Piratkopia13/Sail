#pragma once
#include "Sail/api/IndexBuffer.h"
#include "SVkAPI.h"

class VkIndexBuffer : public IndexBuffer {
public:
	VkIndexBuffer(Mesh::Data& modelData);
	~VkIndexBuffer();

	virtual void bind(void* cmdList) override;

private:
	SVkAPI* m_context;
};

