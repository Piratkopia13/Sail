#include "Model.h"

#include "../shader/basic/SimpleColorShader.h"
#include "../shader/ShaderSet.h"
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

using namespace DirectX::SimpleMath;

Model::Model(Data& buildData, ShaderSet* shaderSet)
	: m_data(buildData)
	, m_aabb(Vector3::Zero, Vector3(.2f, .2f, .2f))
{
	m_material = new Material(shaderSet);
	m_transform = new Transform();
	m_transformChanged = false;

	// Create vertex buffer
	m_vertexBuffer = std::make_unique<VertexBuffer>(shaderSet->getInputLayout(), buildData);
	// Create index buffer is indices are set
	if (buildData.numIndices > 0) {
		m_indexBuffer = std::make_unique<IndexBuffer>(buildData);
	}
}

Model::Model(const std::string& path, ShaderSet* shaderSet)
	/*: m_data(buildData) */
	: m_aabb(Vector3::Zero, Vector3(.2f, .2f, .2f))
{
	m_material = new Material(shaderSet);
	m_transform = new Transform();
	m_transformChanged = false;


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
	Memory::safeDeleteArr(m_data.indices);
	Memory::safeDeleteArr(m_data.positions);
	Memory::safeDeleteArr(m_data.normals);
	Memory::safeDeleteArr(m_data.bitangents);
	Memory::safeDeleteArr(m_data.colors);
	Memory::safeDeleteArr(m_data.tangents);
	Memory::safeDeleteArr(m_data.texCoords);

	Memory::safeDelete(m_material);
	if (!m_transformChanged) {
		Memory::safeDelete(m_transform);
	}
}

//void Model::setBuildData(Data& buildData) {
//	m_data = buildData;
//}
const Model::Data& Model::getBuildData() const {
	return m_data;
}

//void Model::buildBufferForShader(ShaderSet* shader) {
//
//	shader->createBufferFromModelData(&m_vertexBuffer, &m_indexBuffer, &m_instanceBuffer, &m_data);
//	calculateAABB();
//
//}

void Model::draw(Renderer& renderer, bool bindShader) {
	if (m_transform == nullptr) {
		Logger::Error("Model has not been assigned with a transform.");
		return;
	}

	auto* devCon = Application::getInstance()->getAPI()->getDeviceContext();

	//getShader()->setCBufferVar("sys_mWorld", &m_transform->getMatrix().Transpose(), sizeof(Matrix));

	m_material->bind();

	m_vertexBuffer->bind();
	if (m_indexBuffer)
		m_indexBuffer->bind();

	// Set topology
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (m_indexBuffer)
		devCon->DrawIndexed(getNumIndices(), 0U, 0U);
	else
		devCon->Draw(getNumVertices(), 0);


}

UINT Model::getNumVertices() const {
	return m_data.numVertices;
}
UINT Model::getNumIndices() const {
	return m_data.numIndices;
}
UINT Model::getNumInstances() const {
	return m_data.numInstances;
}
const VertexBuffer& Model::getVertexBuffer() const {
	return *m_vertexBuffer;
}
const IndexBuffer& Model::getIndexBuffer() const {
	return *m_indexBuffer;
}

ID3D11Buffer* Model::getInstanceBuffer() const {
	return nullptr; // TODO: fix
}

void Model::setTransform(Transform* newTransform) {
	if (!m_transformChanged) {
		Memory::safeDelete(m_transform);
		m_transformChanged = true;
	}
	m_transform = newTransform;
}

Transform& Model::getTransform() {
	return *m_transform;
}

ShaderSet* Model::getShader() const {
	return m_material->getShader();
}

Material* Model::getMaterial() {
	return m_material;
}

const AABB& Model::getAABB() const {
	return m_aabb;
}
void Model::updateAABB() {
	m_aabb.updateTransform(m_transform->getMatrix());
}

void Model::calculateAABB() {

	Vector3 minCorner(FLT_MAX, FLT_MAX, FLT_MAX );
	Vector3 maxCorner(-FLT_MIN, -FLT_MIN, -FLT_MIN);

	for (UINT i = 0; i < m_data.numVertices; i++) {
		Vector3& p = m_data.positions[i];

		if (p.x < minCorner.x) minCorner.x = p.x;
		if (p.y < minCorner.y) minCorner.y = p.y;
		if (p.z < minCorner.z) minCorner.z = p.z;

		if (p.x > maxCorner.x) maxCorner.x = p.x;
		if (p.y > maxCorner.y) maxCorner.y = p.y;
		if (p.z > maxCorner.z) maxCorner.z = p.z;

	}

	m_aabb.setMinPos(minCorner);
	m_aabb.setMaxPos(maxCorner);

}

void Model::Data::deepCopy(const Data& other) {
	this->numIndices = other.numIndices;
	this->numVertices = other.numVertices;
	this->numInstances = other.numInstances;
	if (other.indices) {
		this->indices = new ULONG[other.numIndices];
		for (UINT i = 0; i < other.numIndices; i++)
			this->indices[i] = other.indices[i];
	}
	UINT numVerts = (other.numIndices > 0) ? other.numIndices : other.numVertices;
	if (other.positions) {
		this->positions = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->positions[i] = other.positions[i];
	}
	if (other.normals) {
		this->normals = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->normals[i] = other.normals[i];
	}
	if (other.colors) {
		this->colors = new DirectX::SimpleMath::Vector4[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->colors[i] = other.colors[i];
	}
	if (other.texCoords) {
		this->texCoords = new DirectX::SimpleMath::Vector2[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->texCoords[i] = other.texCoords[i];
	}
	if (other.tangents) {
		this->tangents = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->tangents[i] = other.tangents[i];
	}
	if (other.bitangents) {
		this->bitangents = new DirectX::SimpleMath::Vector3[numVerts];
		for (UINT i = 0; i < numVerts; i++)
			this->bitangents[i] = other.bitangents[i];
	}
}