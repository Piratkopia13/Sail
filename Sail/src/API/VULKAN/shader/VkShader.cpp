#include "pch.h"
#include "VkShader.h"
#include "../VkAPI.h"
#include "Sail/Application.h"
//#include "VkConstantBuffer.h"
//#include "../resources/DescriptorHeap.h"
#include "../resources/VkTexture.h"
//#include "../resources/VkRenderableTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW VkShader(settings);
	else
		return new (allocAddr) VkShader(settings);
}

VkShader::VkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<VkAPI>();

	assert(false);

	compile();
}

VkShader::~VkShader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);
}

void* VkShader::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
	assert(false);
	return nullptr;
}

void VkShader::instanceFinished() {
	assert(false);
}

void VkShader::reserve(unsigned int meshIndexMax) {
	assert(false);
}

bool VkShader::onEvent(Event& event) {
	assert(false);
	return true;
}

void VkShader::bind(void* cmdList) const {
	bindInternal(getMeshIndex(), cmdList);
}

unsigned int VkShader::getMeshIndex() const {
	assert(false);
	return 0;
}

bool VkShader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	assert(false);
	return true;
}

void VkShader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	assert(false);
}

void VkShader::setCBufferVar(const std::string& name, const void* data, unsigned int size) {
	setCBufferVarInternal(name, data, size, getMeshIndex());
}

bool VkShader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size) {
	return trySetCBufferVarInternal(name, data, size, getMeshIndex());
}