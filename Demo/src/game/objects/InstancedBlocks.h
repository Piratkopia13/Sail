#pragma once

#include "common\Object.h"

template <class T, typename D>
class InstancedBlocks : public Object {
public:
	InstancedBlocks(UINT maxInstances, const std::string& blockModelName = "dynamic_block");
	InstancedBlocks(const std::string& blockModelName = "dynamic_block");
	virtual ~InstancedBlocks();

	virtual void draw();
	// Initializes the model with instance data that correlates to the provided parameter, also reserves memory for instance data
	void init(UINT maxInstances);
	void init();
	void reserve(UINT capacity);
	D& getInstanceData(UINT index);
	D& addInstance(const D& instanceData);

private:
	std::string m_blockModelName;
	std::vector<D> m_instanceData;
	T* m_shader;
};

template <class T, typename D>
InstancedBlocks<T, D>::InstancedBlocks(UINT maxInstances, const std::string& blockModelName) {
	auto* app = Application::getInstance();
	m_blockModelName = blockModelName;

	// Store a pointer to the shader used in rendering
	m_shader = &app->getResourceManager().getShaderSet<T>();

	init(maxInstances);
}

template <class T, typename D>
InstancedBlocks<T, D>::InstancedBlocks(const std::string& blockModelName) {
	auto* app = Application::getInstance();
	m_blockModelName = blockModelName;

	// Store a pointer to the shader used in rendering
	m_shader = &app->getResourceManager().getShaderSet<T>();
}

template <class T, typename D>
InstancedBlocks<T, D>::~InstancedBlocks() {
	delete model;
}

template <class T, typename D>
void InstancedBlocks<T, D>::init(UINT maxInstances) {
	auto* app = Application::getInstance();
	// Use the data from the fbx version
	auto* blockModel = app->getResourceManager().getFBXModel(m_blockModelName).getModel();
	// Add instancing to the build data
	Model::Data buildData;
	buildData.deepCopy(blockModel->getBuildData());
	buildData.numInstances = maxInstances;
	// Create a new model
	model = new Model(buildData);
	model->buildBufferForShader(m_shader);
	// Steal textures from the fbx model
	UINT numTextures;
	model->getMaterial()->setDiffuseTexture(*blockModel->getMaterial()->getTextures(numTextures));

	if (m_instanceData.size() < maxInstances) {
		// Reserve memory to fit max instances
		m_instanceData.reserve(maxInstances);
	}
}

template <class T, typename D>
void InstancedBlocks<T, D>::init() {
	init(m_instanceData.size());
}

template <class T, typename D>
void InstancedBlocks<T, D>::reserve(UINT capacity) {
	m_instanceData.reserve(capacity);
}

template <class T, typename D>
void InstancedBlocks<T, D>::draw() {
	m_shader->updateInstanceData(&m_instanceData[0], m_instanceData.size() * sizeof(m_instanceData[0]), getModel()->getInstanceBuffer());
	getModel()->draw();
}

template <class T, typename D>
D& InstancedBlocks<T, D>::getInstanceData(UINT index) {
	return m_instanceData[index];
}

template <class T, typename D>
D& InstancedBlocks<T, D>::addInstance(const D& instanceData) {
	assert(m_instanceData.size() != m_instanceData.capacity());
	m_instanceData.push_back(instanceData);
	return m_instanceData.back();
}
