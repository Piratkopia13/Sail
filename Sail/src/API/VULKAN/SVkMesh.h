#pragma once

#include "Sail/api/Mesh.h"
#include "SVkAPI.h"

class SVkMesh : public Mesh {
public:
	SVkMesh(Data& buildData);
	~SVkMesh();

	virtual void draw(const Renderer& renderer, Material* material, Shader* shader, void* cmdList) override;

private:
	SVkAPI* m_context;

};