#include "pch.h"
#include "Shader.h"
#include "Sail/utils/Utils.h"

const std::string Shader::DEFAULT_SHADER_LOCATION = "res/shaders/";
unsigned int Shader::s_id = 0;

Shader::Shader(Shaders::ShaderSettings settings)
	: m_filename(settings.filename)
	, parser(settings.filename)
	, m_settings(settings)
	, m_materialType(settings.materialType)
	, m_vsBlob(nullptr)
	, m_psBlob(nullptr)
	, m_dsBlob(nullptr)
	, m_hsBlob(nullptr)
	, m_gsBlob(nullptr)
	, m_csBlob(nullptr)
{
	m_id = s_id++;
}

Shader::~Shader() {
	//delete shaderPipeline;
}

Material::Type Shader::getMaterialType() const {
	return m_materialType;
}

bool Shader::isComputeShader() const {
	return m_csBlob != nullptr;
}

unsigned int Shader::getID() const {
	return m_id;
}

unsigned int Shader::getAttributesHash() const {
	return parser.getParsedData().attributesHash;
}

const Shaders::ShaderSettings& Shader::getSettings() const {
	return m_settings;
}

// Default bind, override and don't call this one if required by the graphics API
void Shader::bind(void* cmdList) const {
	bindInternal(0U, cmdList);
}

RenderableTexture* Shader::getRenderableTexture(const std::string& name) const {
	for (auto& it : parser.getParsedData().renderableTextures) {
		if (it.res.name == name) {
			return it.renderableTexture.get();
		}
	}
	Logger::Error("Tried to get a RenderableTexture named \"" + name + "\" which does not exist in the Shader.");
	return nullptr;
}

// TODO: size isn't really needed, can be read from the byteOffset of the next var
void Shader::setCBufferVar(const std::string& name, const void* data, unsigned int size) {
	setCBufferVarInternal(name, data, size, 0U);
}

bool Shader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size) {
	return trySetCBufferVarInternal(name, data, size, 0U);
}

void* Shader::getVsBlob() const {
	return m_vsBlob;
}

void* Shader::getGsBlob() const {
	return m_gsBlob;
}

void* Shader::getPsBlob() const {
	return m_psBlob;
}

void* Shader::getDsBlob() const {
	return m_dsBlob;
}

void* Shader::getHsBlob() const {
	return m_hsBlob;
}

void* Shader::getCsBlob() const {
	return m_csBlob;
}

void Shader::bindInternal(unsigned int meshIndex, void* cmdList) const {
	for (auto& it : parser.getParsedData().cBuffers) {
		it.cBuffer->bind(meshIndex, cmdList);
	}
	for (auto& it : parser.getParsedData().samplers) {
		it.sampler->bind();
	}
}

void Shader::compile() {
	SAIL_PROFILE_FUNCTION();

	std::string filepath = DEFAULT_SHADER_LOCATION + m_filename;
	std::string source = Utils::readFile(filepath);
	if (source == "")
		Logger::Error("Shader file is empty or does not exist: " + m_filename);
	// Parse source - this might modify it by changing sampler slots etc.
	source = parser.parse(source);
	auto& parsedData = parser.getParsedData();

	if (parsedData.hasVS) {
		m_vsBlob = compileShader(source, filepath, ShaderComponent::VS);
		//Memory::safeRelease(VSBlob); // is this right?
	}
	if (parsedData.hasPS) {
		m_psBlob = compileShader(source, filepath, ShaderComponent::PS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasGS) {
		m_gsBlob = compileShader(source, filepath, ShaderComponent::GS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasDS) {
		m_dsBlob = compileShader(source, filepath, ShaderComponent::DS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasHS) {
		m_hsBlob = compileShader(source, filepath, ShaderComponent::HS);
		//Memory::safeRelease(blob);
	}
	if (parsedData.hasCS) {
		m_csBlob = compileShader(source, filepath, ShaderComponent::CS);
	}
}

void Shader::setMaterialType(Material::Type type) {
	m_materialType = type;
}

bool Shader::trySetCBufferVarInternal(const std::string& name, const void* data, unsigned int size, unsigned int meshIndex) {
	for (auto& it : parser.getParsedData().cBuffers) {
		for (auto& var : it.vars) {
			if (var.name == name) {
				ShaderComponent::ConstantBuffer& cbuffer = *it.cBuffer.get();
				cbuffer.updateData(data, size, meshIndex, var.byteOffset);
				return true;
			}
		}
	}
	return false;
}

void Shader::setCBufferVarInternal(const std::string& name, const void* data, unsigned int size, unsigned int meshIndex) {
	bool success = trySetCBufferVarInternal(name, data, size, meshIndex);
	if (!success)
		Logger::Warning("Tried to set CBuffer variable that did not exist (" + name + ")");
}