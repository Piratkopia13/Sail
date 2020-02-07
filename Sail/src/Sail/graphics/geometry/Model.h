#pragma once

#include <vector>
#include "Sail/api/Mesh.h"
#include "Sail/api/Renderer.h"

class Model {
public:
	typedef std::unique_ptr<Model> Ptr;
	typedef std::shared_ptr<Model> SPtr;

public: 
	Model(const std::string& name);
	Model(Mesh::Data& data, Shader* shader, const std::string& name);
	~Model();

	Mesh* addMesh(std::unique_ptr<Mesh> mesh);

	Mesh* getMesh(unsigned int index);
	unsigned int getNumberOfMeshes() const;
	const std::string& getName() const;

private:
	std::vector<Mesh::Ptr> m_meshes;
	std::string m_name;

};
