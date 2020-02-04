#pragma once

#include "Sail/api/Mesh.h"
#include "DX12API.h"

class DX12Mesh : public Mesh {
public:
	DX12Mesh(Data& buildData, Shader* shader);
	~DX12Mesh();

	virtual void draw(const Renderer& renderer, Material* material, void* cmdList) override;

private:
	DX12API* m_context;

};