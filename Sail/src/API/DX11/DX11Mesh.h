#pragma once

#include "Sail/api/Mesh.h"

class DX11Mesh : public Mesh {
public:
	DX11Mesh(Data& buildData);
	~DX11Mesh();

	virtual void draw(const Renderer& renderer, Material* material, Shader* shader, void* cmdList = nullptr) override;
};