#pragma once

#include <vector>
#include "Sail/api/Mesh.h"
#include "Sail/api/Renderer.h"

class Model {
public:
	typedef std::unique_ptr<Model> Ptr;

public: 
	Model();
	Model(Mesh::Data& data, Shader* shader);
	~Model();

	Mesh* addMesh(std::unique_ptr<Mesh> mesh);

	// Draws the model using its material
	void draw(const Renderer& renderer);

	Mesh* getMesh(unsigned int index);
	unsigned int getNumberOfMeshes() const;

private:
	std::vector<Mesh::Ptr> m_meshes;

};
