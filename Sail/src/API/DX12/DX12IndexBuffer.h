#pragma once
#include "Sail/api/IndexBuffer.h"
#include "DX12API.h"

class DX12IndexBuffer : public IndexBuffer {
public:
	DX12IndexBuffer(Mesh::Data& modelData);
	~DX12IndexBuffer();

	virtual void bind(void* cmdList) const override;

private:
	wComPtr<ID3D12Resource> m_indexBuffer;

};

