#pragma once
#include <d3d11.h>
#include "../shader/InputLayout.h"
#include "../../utils/Utils.h"
#include "Mesh.h"

class VertexBuffer {
public:
	VertexBuffer(const InputLayout& inputLayout, Mesh::Data& modelData);
	~VertexBuffer();

	ID3D11Buffer* const* getBuffer() const;
	void bind() const;

private:
	const InputLayout& m_inputLayout;
	ID3D11Buffer* m_vertBuffer;
	ID3D11Buffer* m_instanceBuffer;
	UINT m_stride;
};

