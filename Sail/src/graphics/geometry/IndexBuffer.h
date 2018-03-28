#pragma once
#include <d3d11.h>
#include "../shader/InputLayout.h"
#include "../../utils/Utils.h"
#include "Model.h"

class IndexBuffer {
public:
	IndexBuffer(Model::Data& modelData);
	~IndexBuffer();

	ID3D11Buffer* const* getBuffer() const;
	void bind() const;

private:
	ID3D11Buffer* m_buffer;
	UINT m_stride;
};

