#pragma once
#include <d3d11.h>
#include "Sail/api/IndexBuffer.h"

class DX11IndexBuffer : public IndexBuffer {
public:
	DX11IndexBuffer(Mesh::Data& modelData);
	~DX11IndexBuffer();

	ID3D11Buffer* const* getBuffer() const;
	virtual void bind(void* cmdList) const override;

private:
	ID3D11Buffer* m_buffer;
};

