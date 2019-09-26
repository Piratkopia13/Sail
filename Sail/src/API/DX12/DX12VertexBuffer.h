#pragma once
#include "Sail/api/VertexBuffer.h"
#include "DX12API.h"

class DX12VertexBuffer : public VertexBuffer {
public:
	DX12VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData);
	~DX12VertexBuffer();

	virtual void bind(void* cmdList) const override;
	ID3D12Resource1* getBuffer() const;
	void update(Mesh::Data& data);
	bool hasBeenUpdated() const;
	void resetHasBeenUpdated();

private:
	DX12API* m_context;
	std::vector<wComPtr<ID3D12Resource1>> m_vertexBuffers;
	std::vector<bool> m_hasBeenUpdated;
};

