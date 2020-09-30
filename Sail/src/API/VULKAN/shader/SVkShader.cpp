#include "pch.h"
#include "SVkShader.h"
#include "../SVkAPI.h"
#include "Sail/Application.h"
#include "SVkConstantBuffer.h"
#include "../SVkUtils.h"
#include "../resources/SVkTexture.h"
#include "SVkSampler.h"
#include "../resources/SVkRenderableTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW SVkShader(settings);
	else
		return new (allocAddr) SVkShader(settings);
}

SVkShader::SVkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
	, m_missingTexture(static_cast<SVkTexture&>(Application::getInstance()->getResourceManager().getTexture(ResourceManager::MISSING_TEXTURE_NAME)))
	, m_missingTextureCube(static_cast<SVkTexture&>(Application::getInstance()->getResourceManager().getTexture(ResourceManager::MISSING_TEXTURECUBE_NAME)))
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	m_renderPass = VK_NULL_HANDLE; // PSO will use default render pass if not set, this can be overridden by setRenderPass before the PSO has been created

	compile();

	// Create the pipeline layout from parsed data
	auto& parsed = parser.getParsedData();
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// Uniform buffer objects in vulkan are used the same way as constant buffers in dx12
	for (auto& cbuffer : parsed.cBuffers) {
		auto& b = bindings.emplace_back();
		b.binding = static_cast<uint32_t>(cbuffer.cBuffer->getSlot());
		b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		b.descriptorCount = 1;
		//b.stageFlags = SVkUtils::ConvertShaderBindingToStageFlags(cbuffer.bindShader); // TODO: find out why this sometimes breaks things
		b.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		b.pImmutableSamplers = nullptr; // Optional
	}

	for (auto& texture : parsed.textures) {
		auto& b = bindings.emplace_back();
		b.binding = static_cast<uint32_t>(texture.vkBinding);
		b.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		b.descriptorCount = (texture.isTexturesArray || texture.isTextureCubesArray) ? TEXTURE_ARRAY_DESCRIPTOR_COUNT : (texture.arraySize == -1) ? 64 : texture.arraySize; // an array size of -1 means it is sizeless in the shader, set some max number
		b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		b.pImmutableSamplers = nullptr; // Optional
	}

	for (auto& sampler : parsed.samplers) {
		auto& b = bindings.emplace_back();
		b.binding = static_cast<uint32_t>(sampler.res.vkBinding);
		b.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		b.descriptorCount = 1;
		b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		b.pImmutableSamplers = nullptr; // Optional
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_context->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout));

	std::vector<VkPushConstantRange> pcRanges;
	for (auto& pc : parser.getParsedData().pushConstants) {
		if (pc.size > 128) {
			Logger::Warning("A push constant is larger than 128, make sure the current device supports this!");
		}
		auto& range = pcRanges.emplace_back();
		range.offset = 0;
		range.size = pc.size;
		range.stageFlags = SVkUtils::ConvertShaderBindingToStageFlags(pc.bindShader);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pcRanges.size());
	pipelineLayoutInfo.pPushConstantRanges = pcRanges.data();

	VK_CHECK_RESULT(vkCreatePipelineLayout(m_context->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

}

SVkShader::~SVkShader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);

	vkDeviceWaitIdle(m_context->getDevice());

	vkDestroyPipelineLayout(m_context->getDevice(), m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_context->getDevice(), m_descriptorSetLayout, nullptr);

	// Clean up shader modules
	auto safeDestroyModule = [&](void* module) {
		if (module) {
			vkDestroyShaderModule(m_context->getDevice(), *static_cast<const VkShaderModule*>(module), nullptr);
			delete module;
		}
	};
	safeDestroyModule(getVsBlob());
	safeDestroyModule(getGsBlob());
	safeDestroyModule(getPsBlob());
	safeDestroyModule(getDsBlob());
	safeDestroyModule(getHsBlob());
	safeDestroyModule(getCsBlob());
}

void* SVkShader::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
	
	std::string path = filepath;

#ifdef _WINDOWS_
	// Try to compile hlsl into spir-v
	if (strcmp(filepath.c_str() + filepath.size() - 4, "hlsl") == 0) {
		Logger::Log("Compiling " + filepath + " into spir-v...");

		std::string stageStr = (shaderType == ShaderComponent::BIND_SHADER::VS) ? "VS" : (shaderType == ShaderComponent::BIND_SHADER::PS)
			? "PS" : (shaderType == ShaderComponent::BIND_SHADER::HS) ? "HS" : (shaderType == ShaderComponent::BIND_SHADER::GS)
			? "GS" : (shaderType == ShaderComponent::BIND_SHADER::DS) ? "DS" : "CS";

		std::stringstream cmd;
		cmd << "..\\shaderconductor\\ShaderConductorCmd.exe";
		cmd << " -E " << stageStr << "Main"; // Entrypoint: VSMain, PSMain, etc.
		cmd << " -I " << filepath;
		std::transform(stageStr.begin(), stageStr.end(), stageStr.begin(), ::tolower); // The next -S argument requires lowercase
		std::string outputPath = filepath;
		outputPath.replace(outputPath.end() - 5, outputPath.end(), "\0");
		outputPath += "-"+stageStr+".spv";
		cmd << " -O " << outputPath;
		cmd << " -S " << stageStr; // vs, ps, etc.
		cmd << " -T spirv";

		system(cmd.str().c_str());

		// The outputed spir-v file is the one we want to read
		path = outputPath;
	}
#else
#endif

	auto shaderCode = Utils::readFileBinary(path);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = shaderCode.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

	// TODO: Shader modules are alive for the whole lifetime of SVkShader, consider if this is really required
	VkShaderModule* shaderModule = SAIL_NEW VkShaderModule;
	if (vkCreateShaderModule(m_context->getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		Logger::Error("Failed to create shader module!");
	}

	return shaderModule;
}

bool SVkShader::onEvent(Event& event) {
	auto newFrame = [&](NewFrameEvent& event) {
		// Clear frame dependent resources
		m_uniqueMaterials.clear();
		m_uniqueTextures.clear();
		m_uniqueTextureCubes.clear();
		m_lastMaterialIndex = 0;
		m_lastTextureIndex = 0;
		m_lastTextureCubeIndex = 0;
		return true;
	};
	EventHandler::HandleType<NewFrameEvent>(event, newFrame);
	return true;
}

void SVkShader::prepareToRender(std::vector<Renderer::RenderCommand>& renderCommands, const SVkPipelineStateObject* pso) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shader prepareToRender");
	// If the shader wants all textures this frame in an array

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
		for (auto* texture : mat->getTextures()) {
			if (texture) {
				SVkTexture* tex = static_cast<SVkTexture*>(texture);
				auto& uniqueTexs = (tex->isCubeMap()) ? m_uniqueTextureCubes : m_uniqueTextures;
				auto& texIndex = (tex->isCubeMap()) ? m_lastTextureCubeIndex : m_lastTextureIndex;

				auto it = std::find(uniqueTexs.begin(), uniqueTexs.end(), tex);
				if (it == uniqueTexs.end()) {
					uniqueTexs.emplace_back(tex);
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
				SVkRenderableTexture* tex = static_cast<SVkRenderableTexture*>(rendTexture);
				auto it = std::find(m_uniqueTextures.begin(), m_uniqueTextures.end(), tex);
				if (it == m_uniqueTextures.end()) {
					m_uniqueTextures.emplace_back(tex);
				}
			}
		}
	}

	// Find what slot, if any, should be used to bind all textures
	unsigned int textureArrBinding = 0;
	unsigned int textureCubeArrBinding = 0;
	bool shouldBindTextureArr = false;
	bool shouldBindTextureCubeArr = false;
	for (auto& tex : parser.getParsedData().textures) {
		if (tex.isTexturesArray) {
			textureArrBinding = tex.vkBinding;
			shouldBindTextureArr = true;
		} else if (tex.isTextureCubesArray) {
			textureCubeArrBinding = tex.vkBinding;
			shouldBindTextureCubeArr = true;
		}
		if (shouldBindTextureArr && shouldBindTextureCubeArr) break; // Only one slot can be used for each array binding, early exit
	}

	// Find which buffer, if any, should contain materials
	ShaderComponent::SVkConstantBuffer* materialBuffer = nullptr;;
	unsigned int materialBinding = 0;
	for (auto& cbuffer : parser.getParsedData().cBuffers) {
		if (cbuffer.isMaterialArray) {
			materialBuffer = static_cast<ShaderComponent::SVkConstantBuffer*>(cbuffer.cBuffer.get());
			materialBinding = materialBuffer->getSlot();
			break;
		}
	}

	// Write materials to the buffer
	unsigned int newMaterials = m_lastMaterialIndex - materialIndexStart;
	if (materialBuffer && !m_uniqueMaterials.empty() && newMaterials > 0) {
		// Each shader can only support one material, which means that the size of each is the same
		auto dataSize = m_uniqueMaterials[0]->getDataSize();

		for (unsigned int i = materialIndexStart; i < materialIndexStart+newMaterials; i++) {
			materialBuffer->updateData(m_uniqueMaterials[i]->getData(), dataSize, 0, i*dataSize);
		}
	}

	VkWriteDescriptorSet descWrite[] = { {}, {}, {} };

	auto imageIndex = m_context->getSwapImageIndex();
	// Create descriptors for the 2D texture array
	if (shouldBindTextureArr) {
		std::vector<VkDescriptorImageInfo> imageInfos;
		for (auto* texture : m_uniqueTextures) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (texture->isReadyToUse()) {
				imageInfo.imageView = texture->getView();
			} else {
				imageInfo.imageView = m_missingTexture.getView();
			}
		}

		if (imageInfos.size() > TEXTURE_ARRAY_DESCRIPTOR_COUNT) {
			Logger::Error("Tried to render too many 2D textures (" + std::to_string(imageInfos.size()) + "). Set max/TEXTURE_ARRAY_DESCRIPTOR_COUNT is " + std::to_string(TEXTURE_ARRAY_DESCRIPTOR_COUNT));
		}

		// Fill the unused descriptor slots with the missing texture
		for (unsigned int i = imageInfos.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_missingTexture.getView();
		}

		descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[0].dstSet = pso->getDescriptorSet();
		descWrite[0].dstBinding = textureArrBinding;
		descWrite[0].dstArrayElement = 0;
		descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		//descWrite[0].descriptorCount = static_cast<uint32_t>(imageInfos.size());
		descWrite[0].descriptorCount = TEXTURE_ARRAY_DESCRIPTOR_COUNT;
		descWrite[0].pImageInfo = imageInfos.data();

		if (imageInfos.empty()) return;
		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[0], 0, nullptr);
	}

	// Create descriptors for the texture cube array
	if (shouldBindTextureCubeArr) {
		std::vector<VkDescriptorImageInfo> imageInfos;
		for (auto* texture : m_uniqueTextureCubes) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (texture->isReadyToUse()) {
				imageInfo.imageView = texture->getView();
			} else {
				// Use a temporary texture cube
				imageInfo.imageView = m_missingTextureCube.getView();
			}
		}

		if (imageInfos.size() > TEXTURE_ARRAY_DESCRIPTOR_COUNT) {
			Logger::Error("Tried to render too many cube textures (" + std::to_string(imageInfos.size()) + "). Set max/TEXTURE_ARRAY_DESCRIPTOR_COUNT is " + std::to_string(TEXTURE_ARRAY_DESCRIPTOR_COUNT));
		}

		// Fill the unused descriptor slots with the missing texture
		for (unsigned int i = imageInfos.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_missingTextureCube.getView();
		}

		descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[1].dstSet = pso->getDescriptorSet();
		descWrite[1].dstBinding = textureCubeArrBinding;
		descWrite[1].dstArrayElement = 0;
		descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descWrite[1].descriptorCount = TEXTURE_ARRAY_DESCRIPTOR_COUNT;
		descWrite[1].pImageInfo = imageInfos.data();

		if (imageInfos.empty()) return;
		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[1], 0, nullptr);
	}

	// Create descriptor for materials
	if (materialBuffer)	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = materialBuffer->getBuffer(imageIndex);
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		
		descWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[2].dstSet = pso->getDescriptorSet();
		descWrite[2].dstBinding = materialBinding;
		descWrite[2].dstArrayElement = 0;
		descWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWrite[2].descriptorCount = 1;
		descWrite[2].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[2], 0, nullptr);
	}
}

const VkPipelineLayout& SVkShader::getPipelineLayout() const {
	return m_pipelineLayout;
}

void SVkShader::setRenderPass(const VkRenderPass& renderPass) {
	m_renderPass = renderPass;
}

const VkRenderPass& SVkShader::getRenderPass() const {
	return m_renderPass;
}

void SVkShader::bind(void* cmdList) const {
	auto imageIndex = m_context->getSwapImageIndex();
	bindInternal(0U, cmdList);
}

void SVkShader::recompile() {
	vkDeviceWaitIdle(m_context->getDevice());
	// Clean up shader modules
	auto safeDestroyModule = [&](void* module) {
		if (module) {
			vkDestroyShaderModule(m_context->getDevice(), *static_cast<const VkShaderModule*>(module), nullptr);
			delete module;
		}
	};
	safeDestroyModule(getVsBlob());
	safeDestroyModule(getGsBlob());
	safeDestroyModule(getPsBlob());
	safeDestroyModule(getDsBlob());
	safeDestroyModule(getHsBlob());
	safeDestroyModule(getCsBlob());

	parser.clearParsedData();
	compile();
}

bool SVkShader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	//if (!texture) return false; // No texture bound to this slot
	//auto* vkTexture = static_cast<SVkTexture*>(texture);
	
	assert(false && "Using setTexture is not supported in Vulkan. Place the texture in a material instead, and it will be set in the shader automatically.");

	return true;
}

void SVkShader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	assert(false);
}

void SVkShader::setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	if (!trySetPushConstant(name, data, size, cmdList)) {
		setCBufferVarInternal(name, data, size, 0U);;
	}
}

bool SVkShader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	if (!trySetPushConstant(name, data, size, cmdList)) {
		return trySetCBufferVarInternal(name, data, size, 0U);
	}
	return true;
}

bool SVkShader::trySetPushConstant(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	// Check if name matches a push constant
	bool wasPushConstant = false;
	for (auto& pc : parser.getParsedData().pushConstants) {
		for (auto& var : pc.vars) {
			if (var.name == name) {
				// Set the push constant
				vkCmdPushConstants(static_cast<VkCommandBuffer>(cmdList), m_pipelineLayout, SVkUtils::ConvertShaderBindingToStageFlags(pc.bindShader), var.byteOffset, size, data);

				wasPushConstant = true;
				break;
			}
		}
	}
	return wasPushConstant;
}

void SVkShader::createDescriptorSet(std::vector<VkDescriptorSet>& outDescriptorSet) const {
	// Create one descriptor set per swap image

	auto& parsed = parser.getParsedData();

	auto numBuffers = m_context->getNumSwapchainImages();
	std::vector<VkDescriptorSetLayout> layouts(numBuffers, m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_context->getDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numBuffers);
	allocInfo.pSetLayouts = layouts.data();

	outDescriptorSet.resize(numBuffers);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_context->getDevice(), &allocInfo, outDescriptorSet.data()));

	// Write descriptors to cbuffers that do not need their their descriptors updated during rendering
	for (size_t i = 0; i < numBuffers; i++) {
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		std::vector<VkDescriptorBufferInfo> cbufferInfos;
		cbufferInfos.reserve(parsed.cBuffers.size());
		for (auto& buffer : parsed.cBuffers) {
			if (buffer.isMaterialArray) continue; // material arrays will be updated in prepareToRender()

			auto* svkBuffer = static_cast<ShaderComponent::SVkConstantBuffer*>(buffer.cBuffer.get());

			auto& info = cbufferInfos.emplace_back();
			info.buffer = svkBuffer->getBuffer(i);
			info.offset = 0;
			info.range = VK_WHOLE_SIZE;

			auto& desc = writeDescriptors.emplace_back();
			desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc.dstSet = outDescriptorSet[i];
			desc.dstBinding = svkBuffer->getSlot();
			desc.dstArrayElement = 0;
			desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			desc.descriptorCount = 1; // One binding for each constant buffer / ubo
			desc.pBufferInfo = &info;
		}

		std::vector<VkDescriptorImageInfo> samplerInfos;
		samplerInfos.reserve(parsed.samplers.size());
		for (auto& sampler : parsed.samplers) {
			auto& info = samplerInfos.emplace_back();
			info.sampler = static_cast<ShaderComponent::SVkSampler*>(sampler.sampler.get())->get();

			auto& desc = writeDescriptors.emplace_back();
			desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc.dstSet = outDescriptorSet[i];
			desc.dstBinding = sampler.res.vkBinding;
			desc.dstArrayElement = 0;
			desc.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			desc.descriptorCount = 1; // One binding for each sampler
			desc.pImageInfo = &info;
		}

		// Write descriptors for this frame index
		// NOTE: All used descriptorInfos must be kept in memory when this is called
		if (!writeDescriptors.empty())
			vkUpdateDescriptorSets(m_context->getDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}
}
