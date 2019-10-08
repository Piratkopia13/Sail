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
	void init(ID3D12GraphicsCommandList4* cmdList);

private:
	DX12API* m_context;
	unsigned int m_byteSize;
	std::vector<wComPtr<ID3D12Resource1>> m_uploadVertexBuffers;
	std::vector<wComPtr<ID3D12Resource1>> m_defaultVertexBuffers;
	std::vector<bool> m_hasBeenInitialized;
	std::vector<bool> m_hasBeenUpdated;
};

