#pragma once

#include "Sail/api/Mesh.h"
#include "DX12API.h"

class DX12Mesh : public Mesh {
public:
	DX12Mesh(Data& buildData, Shader* shader);
	~DX12Mesh();

	/*[Depricated]*/
	virtual void draw(const Renderer& renderer, void* cmdList) override;
	virtual void draw_new(const Renderer& renderer, void* cmdList, int meshIndex);
	virtual void bindMaterial(void* cmdList, int meshIndex);

private:
	DX12API* m_context;

};