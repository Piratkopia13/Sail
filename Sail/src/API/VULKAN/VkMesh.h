#pragma once

#include "Sail/api/Mesh.h"
#include "VkAPI.h"

class VkMesh : public Mesh {
public:
	VkMesh(Data& buildData);
	~VkMesh();

	virtual void draw(const Renderer& renderer, Material* material, Shader* shader, Environment* environment, void* cmdList) override;

private:
	VkAPI* m_context;

};