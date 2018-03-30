#include "Model.h"

#include "../shader/basic/SimpleColorShader.h"
#include "../shader/ShaderSet.h"
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace DirectX::SimpleMath;

Model::Model(Mesh::Data& buildData, ShaderSet* shaderSet) {

	m_mesh = std::make_shared<Mesh>(buildData, shaderSet);

	// TODO: reuse materials (?)
	m_material = std::make_shared<Material>(shaderSet);
}

Model::Model(const std::string& path, ShaderSet* shaderSet) {
	// TODO: reuse mesh if it has already been loaded
	// TODO: load mesh from FBX model specified by path
	//m_mesh = std::make_shared<Mesh>(buildData, shaderSet);

	// TODO: reuse materials (?)
	m_material = std::make_shared<Material>(shaderSet);
}

//Model::Model(ShaderSet* shaderSet)
//	: m_aabb(Vector3::Zero, Vector3(.2f, .2f, .2f))
//	, m_vertexBuffer(nullptr)
//	, m_indexBuffer(nullptr)
//	, m_shader(shaderSet)
//{
//	m_material = new Material();
//	m_transform = new Transform();
//	m_transformChanged = false;
//}
Model::~Model() {
}

//void Model::setBuildData(Data& buildData) {
//	m_data = buildData;
//}
//const Model::Data& Model::getBuildData() const {
//	return m_data;
//}

//void Model::buildBufferForShader(ShaderSet* shader) {
//
//	shader->createBufferFromModelData(&m_vertexBuffer, &m_indexBuffer, &m_instanceBuffer, &m_data);
//	calculateAABB();
//
//}

void Model::draw(Renderer& renderer) {

	m_material->bind();
	m_mesh->draw();

}

ShaderSet* Model::getShader() const {
	return m_material->getShader();
}

Material* Model::getMaterial() {
	return m_material.get();
}

//const AABB& Model::getAABB() const {
//	return m_aabb;
//}
//void Model::updateAABB() {
//	m_aabb.updateTransform(m_transform->getMatrix());
//}
//
//void Model::calculateAABB() {
//
//	Vector3 minCorner(FLT_MAX, FLT_MAX, FLT_MAX );
//	Vector3 maxCorner(-FLT_MIN, -FLT_MIN, -FLT_MIN);
//
//	for (UINT i = 0; i < m_data.numVertices; i++) {
//		Vector3& p = m_data.positions[i];
//
//		if (p.x < minCorner.x) minCorner.x = p.x;
//		if (p.y < minCorner.y) minCorner.y = p.y;
//		if (p.z < minCorner.z) minCorner.z = p.z;
//
//		if (p.x > maxCorner.x) maxCorner.x = p.x;
//		if (p.y > maxCorner.y) maxCorner.y = p.y;
//		if (p.z > maxCorner.z) maxCorner.z = p.z;
//
//	}
//
//	m_aabb.setMinPos(minCorner);
//	m_aabb.setMaxPos(maxCorner);
//
//}
