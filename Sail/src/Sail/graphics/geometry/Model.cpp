#include "pch.h"
#include "Model.h"

Model::Model(Mesh::Data& buildData, Shader* shader) {
	// TODO: reuse mesh if it has already been loaded
	m_meshes.push_back(std::unique_ptr<Mesh>(Mesh::Create(buildData, shader)));
}

Model::Model() {
}

Model::~Model() {
}

Mesh* Model::addMesh(std::unique_ptr<Mesh> mesh) {
	m_meshes.push_back(std::move(mesh));
	return m_meshes.back().get();
}

Mesh* Model::getMesh(unsigned int index) {
	assert(m_meshes.size() > index);
	return m_meshes[index].get();
}

unsigned int Model::getNumberOfMeshes() const {
	return static_cast<unsigned int>(m_meshes.size());
}
