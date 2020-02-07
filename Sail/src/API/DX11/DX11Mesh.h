#pragma once

#include "Sail/api/Mesh.h"

class DX11Mesh : public Mesh {
public:
	DX11Mesh(Data& buildData, Shader* shader);
	~DX11Mesh();

	virtual void draw(const Renderer& renderer, Material* material, Environment* environment, void* cmdList) override;

};