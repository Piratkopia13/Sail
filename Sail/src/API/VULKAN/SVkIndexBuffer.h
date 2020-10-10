#pragma once
#include "Sail/api/IndexBuffer.h"
#include "SVkAPI.h"

class SVkIndexBuffer : public IndexBuffer {
public:
	SVkIndexBuffer(Mesh::Data& modelData);
	~SVkIndexBuffer();

	virtual void bind(void* cmdList) override;

private:
	SVkAPI* m_context;

	// Staging buffers are only used when allowUpdates == false
	// They are destroyed as soon as the data is in the device local indexBuffer
	SVkAPI::BufferAllocation m_stagingBuffer;

	SVkAPI::BufferAllocation m_indexBuffer;
};

