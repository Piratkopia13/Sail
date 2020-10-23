#include "pch.h"
#include "Shader.h"
#include "Sail/utils/Utils.h"
#include "Sail/Application.h"

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
	, m_rgenBlob(nullptr)
	, m_rchitBlob(nullptr)
	, m_rmissBlob(nullptr)
	, m_lastMaterialIndex(0)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_id = s_id++;
}

Shader::~Shader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);
}

Material::Type Shader::getMaterialType() const {
	return m_materialType;
}

bool Shader::isComputeShader() const {
	return m_csBlob != nullptr;
}

bool Shader::isRayTracingShader() const {
	return m_rgenBlob != nullptr;
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
	bindInternal(cmdList);
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

void Shader::setCBuffer(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	// TODO: make this look like the other set methods
	// TODO: make a trySet version
	for (auto& it : parser.getParsedData().cBuffers) {
		if (it.name == name) {
			ShaderComponent::ConstantBuffer& cbuffer = *it.cBuffer.get();
			cbuffer.updateData(data, size);
			return;
		}
	}
	Logger::Warning("Tried to set CBuffer that did not exist (" + name + ")");
}

// TODO: size isn't really needed, can be read from the byteOffset of the next var
void Shader::setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	setCBufferVarInternal(name, data, size);
}

bool Shader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	return trySetCBufferVarInternal(name, data, size);
}

void Shader::setConstantVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	bool success = Shader::trySetConstantVar(name, data, size, cmdList);
	if (!success)
		Logger::Warning("Tried to set CBuffer variable that did not exist (" + name + ")");
}

bool Shader::trySetConstantVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	// Set as constant if it is a constant
	uint32_t offset;
	ShaderComponent::BIND_SHADER bindShader;
	if (findConstant(name, &offset, &bindShader)) {
		setConstantDerived(name, data, size, bindShader, offset, cmdList);
		return true;
	}
	return false;
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

void* Shader::getRayGenBlob() const {
	return m_rgenBlob;
}

void* Shader::getRayCHitBlob() const {
	return m_rchitBlob;
}

void* Shader::getRayMissBlob() const {
	return m_rmissBlob;
}

const ShaderParser::ParsedData& Shader::getParsedData() const {
	return parser.getParsedData();
}

bool Shader::onEvent(Event& event) {
	auto newFrame = [&](NewFrameEvent& event) {
		// Clear frame dependent resources
		m_uniqueMaterials.clear();
		m_lastMaterialIndex = 0;
		return true;
	};
	EventHandler::HandleType<NewFrameEvent>(event, newFrame);
	return true;
}

void Shader::bindInternal(void* cmdList) const {
	for (auto& it : parser.getParsedData().cBuffers) {
		it.cBuffer->bind(cmdList);
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
	if (parsedData.hasRayGen) {
		m_rgenBlob = compileShader(source, filepath, ShaderComponent::RAY_GEN);
	}
	if (parsedData.hasRayClosestHit) {
		m_rchitBlob = compileShader(source, filepath, ShaderComponent::RAY_CLOSEST_HIT);
	}
	if (parsedData.hasRayMiss) {
		m_rmissBlob = compileShader(source, filepath, ShaderComponent::RAY_MISS);
	}
}

bool Shader::findConstant(const std::string& name, uint32_t* outOffset, ShaderComponent::BIND_SHADER* outBindShader) const {
	// Check if name matches a push constant
	bool wasPushConstant = false;
	for (auto& pc : parser.getParsedData().constants) {
		for (auto& var : pc.vars) {
			if (var.name == name) {
				*outBindShader = pc.bindShader;
				*outOffset = var.byteOffset;
				wasPushConstant = true;
				break;
			}
		}
	}
	return wasPushConstant;
}

void Shader::setMaterialType(Material::Type type) {
	m_materialType = type;
}

bool Shader::trySetCBufferVarInternal(const std::string& name, const void* data, unsigned int size) {
	for (auto& it : parser.getParsedData().cBuffers) {
		for (auto& var : it.vars) {
			if (var.name == name) {
				ShaderComponent::ConstantBuffer& cbuffer = *it.cBuffer.get();
				cbuffer.updateData(data, size, var.byteOffset);
				return true;
			}
		}
	}
	return false;
}

void Shader::setCBufferVarInternal(const std::string& name, const void* data, unsigned int size) {
	bool success = Shader::trySetCBufferVarInternal(name, data, size);
	if (!success)
		Logger::Warning("Tried to set CBuffer variable that did not exist (" + name + ")");
}

void Shader::getDescriptorUpdateInfoAndUpdateMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, DescriptorUpdateInfo* outUpdateInfo) {
	auto swapIndex = Application::getInstance()->getAPI()->getSwapIndex();

	unsigned int lastTextureIndex = 0;
	unsigned int lastTextureCubeIndex = 0;

	unsigned int materialIndexStart = m_lastMaterialIndex;
	// Iterate render commands and place uniques materials into the list, and update their index
	for (auto& command : renderCommands) {
		Material* mat = command.material;

		auto it = std::find(m_uniqueMaterials.begin(), m_uniqueMaterials.end(), mat);
		if (it == m_uniqueMaterials.end()) {
			m_uniqueMaterials.emplace_back(mat);
			command.materialIndex = m_lastMaterialIndex++;
		} else {
			command.materialIndex = it - m_uniqueMaterials.begin(); // Get index of the iterator
		}
	}

	// Iterate unique materials and place unique textures and textureCubes into the list, and update their index
	for (auto& mat : m_uniqueMaterials) {
		unsigned int i = 0;
		// Set the environment to use
		// This could update textures and has to be done before textures are extracted
		mat->setEnvironment(environment);

		for (auto* texture : mat->getTextures()) {
			if (texture) {
				auto& uniqueTexs = (texture->isCubeMap()) ? outUpdateInfo->uniqueTextureCubes : outUpdateInfo->uniqueTextures;
				auto& texIndex = (texture->isCubeMap()) ? lastTextureCubeIndex : lastTextureIndex;

				auto it = std::find(uniqueTexs.begin(), uniqueTexs.end(), texture);
				if (it == uniqueTexs.end()) {
					uniqueTexs.emplace_back(texture);
					mat->setTextureIndex(i, texIndex++);
				} else {
					mat->setTextureIndex(i, it - uniqueTexs.begin()); // Get the index of the iterator
				}
			} else {
				mat->setTextureIndex(i, -1); // No texture bound, set index to -1
			}
			i++;
		}
		// Handle renderable texture
		// TODO: add support for materials to use indices for renderable textures
		for (auto* rendTexture : mat->getRenderableTextures()) {
			if (rendTexture) {
				auto it = std::find(outUpdateInfo->uniqueRenderableTextures.begin(), outUpdateInfo->uniqueRenderableTextures.end(), rendTexture);
				if (it == outUpdateInfo->uniqueRenderableTextures.end()) {
					outUpdateInfo->uniqueRenderableTextures.emplace_back(rendTexture);
				}
			}
		}
	}

	assert(outUpdateInfo->uniqueTextures.size() + outUpdateInfo->uniqueRenderableTextures.size() <= TEXTURE_ARRAY_DESCRIPTOR_COUNT && "Tried to render too many 2D textures");
	assert(outUpdateInfo->uniqueTextureCubes.size() <= TEXTURE_ARRAY_DESCRIPTOR_COUNT && "Tried to render too many cube textures");

	// Find what slot, if any, should be used to bind all textures

	for (auto& tex : parser.getParsedData().textures) {
		if (tex.isTexturesArray) {
			outUpdateInfo->textureArrayBinding = tex.res.vkBinding;
			outUpdateInfo->bindTextureArray = true;
			outUpdateInfo->textureArrayIsWritable = tex.isWritable;
		} else if (tex.isTextureCubesArray) {
			outUpdateInfo->textureCubeArrayBinding = tex.res.vkBinding;
			outUpdateInfo->bindTextureCubeArray = true;
			outUpdateInfo->textureCubeArrayIsWritable = tex.isWritable;
		}
		if (outUpdateInfo->bindTextureArray && outUpdateInfo->bindTextureCubeArray) break; // Only one slot can be used for each array binding, early exit
	}

	// Find which buffer, if any, should contain materials
	
	for (auto& cbuffer : parser.getParsedData().cBuffers) {
		if (cbuffer.isMaterialArray) {
			outUpdateInfo->materialBuffer = cbuffer.cBuffer.get();
			break;
		}
	}

	// Write materials to the buffer
	unsigned int newMaterials = m_lastMaterialIndex - materialIndexStart;
	if (outUpdateInfo->materialBuffer && !m_uniqueMaterials.empty() && newMaterials > 0) {
		// Each shader can only support one material, which means that the size of each is the same
		auto dataSize = m_uniqueMaterials[0]->getDataSize();

		for (unsigned int i = materialIndexStart; i < materialIndexStart + newMaterials; i++) {
			outUpdateInfo->materialBuffer->updateData(m_uniqueMaterials[i]->getData(), dataSize, i * dataSize);
		}
	}
}
