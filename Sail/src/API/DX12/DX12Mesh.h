#pragma once

#include <d3d12.h>
#include "Sail/api/Mesh.h"

class DX12Mesh : public Mesh {
public:
	DX12Mesh(Data& buildData, ShaderSet* shaderSet);
	~DX12Mesh();

	virtual void draw(const Renderer& renderer) override;

private:

};