#pragma once

#include <d3d11.h>
#include "Sail/api/Mesh.h"

class DX11Mesh : public Mesh {
public:
	DX11Mesh(Data& buildData, ShaderSet* shaderSet);
	~DX11Mesh();

	virtual void draw(const Renderer& renderer) override;

private:

};