#pragma once
#include "Sail/api/IndexBuffer.h"
#include "DX12API.h"

class DX12IndexBuffer : public IndexBuffer {
public:
	DX12IndexBuffer(Mesh::Data& modelData);
	~DX12IndexBuffer();

	virtual void bind(void* cmdList) const override;
	ID3D12Resource* getBuffer() const;

	unsigned int getByteSize() const override;

private:
	DX12API* m_context;
	std::vector<wComPtr<ID3D12Resource>> m_indexBuffers;

};

