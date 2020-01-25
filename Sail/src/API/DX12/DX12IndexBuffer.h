#pragma once
#include "Sail/api/IndexBuffer.h"
#include "DX12API.h"

class DX12IndexBuffer : public IndexBuffer {
public:
	DX12IndexBuffer(Mesh::Data& modelData);
	~DX12IndexBuffer();

	virtual void bind(void* cmdList) override;
	ID3D12Resource* getBuffer() const;
	bool init(ID3D12GraphicsCommandList4* cmdList);

private:
	DX12API* m_context;
	unsigned int m_byteSize;
	
	wComPtr<ID3D12Resource> m_uploadIndexBuffers;

	wComPtr<ID3D12Resource> m_defaultIndexBuffers;
	bool m_hasBeenInitialized;

	DX12API::CommandQueue* m_queueUsedForUpload;
	UINT64 m_initFenceVal;
	unsigned int m_initFrameCount;
};

