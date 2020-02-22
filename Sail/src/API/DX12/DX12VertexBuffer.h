#pragma once
#include "Sail/api/VertexBuffer.h"
#include "DX12API.h"

class DX12VertexBuffer : public VertexBuffer {
public:
	DX12VertexBuffer(const Mesh::Data& modelData, bool allowUpdates = false);
	~DX12VertexBuffer();

	virtual void bind(void* cmdList) override;
	bool init(ID3D12GraphicsCommandList4* cmdList);
	void update(Mesh::Data& data);
	
	ID3D12Resource* getResource() const;
	// TODO: make these less methods into better ones that doesn't require renderers to call reset
	void setAsUpdated();
	bool hasBeenUpdated() const;
	void resetHasBeenUpdated();

private:
	DX12API* m_context;
	bool m_allowUpdates;
	unsigned int m_byteSize;

	std::vector<wComPtr<ID3D12Resource>> m_uploadVertexBuffers;
	DX12API::CommandQueue* m_queueUsedForUpload;
	UINT64 m_initFenceVal;
	unsigned int m_initFrameCount;

	std::vector<wComPtr<ID3D12Resource>> m_defaultVertexBuffers;
	std::vector<bool> m_hasBeenInitialized;
	std::vector<bool> m_hasBeenUpdated;
};