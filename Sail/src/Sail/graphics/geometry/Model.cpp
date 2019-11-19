#include "pch.h"
#include "Model.h"

Model::Model(Mesh::Data& buildData, Shader* shader) 
	: m_isAnimated(false)
	, m_castShadows(true)
{
	m_meshes.push_back(std::unique_ptr<Mesh>(Mesh::Create(buildData, shader)));
	m_totalByteSize = sizeof(this) + m_meshes[0]->getByteSize();
}

Model::Model(unsigned int numVertices, Shader* shader) 
	: m_isAnimated(false)
{
	m_meshes.push_back(std::unique_ptr<Mesh>(Mesh::Create(numVertices, shader)));
	m_totalByteSize = sizeof(this) + m_meshes[0]->getByteSize();
}
Model::Model() 
	: m_isAnimated(false) 
{ 
	m_totalByteSize = sizeof(this);
}

Model::~Model() {
}

void Model::setName(const std::string& name) {
}

Mesh* Model::addMesh(std::unique_ptr<Mesh> mesh) {
	m_meshes.push_back(std::move(mesh));
	m_totalByteSize += mesh->getByteSize();
	return m_meshes.back().get();
}

void Model::draw(const Renderer& renderer) {
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

unsigned int Model::getByteSize() const {
	return m_totalByteSize;
}

void Model::setIsAnimated(bool animated) {
	m_isAnimated = animated;
}

bool Model::isAnimated() const {
	return m_isAnimated;
}

void Model::setCastShadows(bool cast) {
	m_castShadows = cast;
}

bool Model::castsShadows() const {
	return m_castShadows;
}
