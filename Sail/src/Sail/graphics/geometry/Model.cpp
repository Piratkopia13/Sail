#include "pch.h"
#include "Model.h"

Model::Model(Mesh::Data& buildData, Shader* shader) 
	: m_isAnimated(false)
{
	m_meshes.push_back(std::unique_ptr<Mesh>(Mesh::Create(buildData, shader)));
}

Model::Model() 
	: m_isAnimated(false) 
{ }

Model::~Model() {
}

Mesh* Model::addMesh(std::unique_ptr<Mesh> mesh) {
	m_meshes.push_back(std::move(mesh));
	return m_meshes.back().get();
}

void Model::draw(const Renderer& renderer) {
	//m_material->bind();
	for (auto& mesh : m_meshes)
		mesh->draw(renderer);
}

Mesh* Model::getMesh(unsigned int index) {
	assert(m_meshes.size() > index);
	return m_meshes[index].get();
}

UINT Model::getNumberOfMeshes() const {
	return static_cast<UINT>(m_meshes.size());
}

void Model::setIsAnimated(bool animated) {
	m_isAnimated = animated;
}

bool Model::isAnimated() const {
	return m_isAnimated;
}
