#pragma once
#include <d3d11.h>
#include "shader/DX11InputLayout.h"
#include "Sail/api/VertexBuffer.h"

class DX11VertexBuffer : public VertexBuffer {
public:
	DX11VertexBuffer(const InputLayout& inputLayout, const Mesh::Data& modelData, bool allowUpdates = false);
	~DX11VertexBuffer();

	ID3D11Buffer* const* getBuffer() const;
	virtual void bind(void* cmdList) override;
	virtual void update(Mesh::Data& data) override;

private:
	ID3D11Buffer* m_vertBuffer;
	ID3D11Buffer* m_instanceBuffer;
};

