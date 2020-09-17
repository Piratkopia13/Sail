#include "pch.h"
#include "SVkShader.h"
#include "../SVkAPI.h"
#include "Sail/Application.h"
#include "SVkConstantBuffer.h"
#include "../SVkUtils.h"
#include "../resources/SVkTexture.h"
#include "SVkSampler.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW SVkShader(settings);
	else
		return new (allocAddr) SVkShader(settings);
}

SVkShader::SVkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
	, m_missingTexture(static_cast<SVkTexture&>(Application::getInstance()->getResourceManager().getTexture(ResourceManager::MISSING_TEXTURE_NAME)))
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<SVkAPI>();

	compile();

	// Create the pipeline layout from parsed data
	auto& parsed = parser.getParsedData();
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	// Uniform buffer objects in vulkan are used the same way as constant buffers in dx12
	auto& cBuffers = parsed.cBuffers;
	for (auto& cbuffer : cBuffers) {
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
		b.descriptorCount = (texture.isTexturesArray) ? TEXTURE_ARRAY_DESCRIPTOR_COUNT : (texture.arraySize == -1) ? 64 : texture.arraySize; // an array size of -1 means it is sizeless in the shader, set some max number
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

	// Create one descriptor set per swap image

	auto numBuffers = m_context->getNumSwapChainImages();
	std::vector<VkDescriptorSetLayout> layouts(numBuffers, m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_context->getDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numBuffers);
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(numBuffers);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_context->getDevice(), &allocInfo, m_descriptorSets.data()));

	// Write descriptors to cbuffers that do not need their their descriptors updated during rendering
	std::vector<VkWriteDescriptorSet> writeDescriptors;
	for (size_t i = 0; i < numBuffers; i++) {
		for (auto& buffer : cBuffers) {
			if (buffer.isMaterialArray) continue; // material arrays will be updated in prepareToRender()

			auto* svkBuffer = static_cast<ShaderComponent::SVkConstantBuffer*>(buffer.cBuffer.get());
			
			VkDescriptorBufferInfo info{};
			info.buffer = svkBuffer->getBuffer(i);
			info.offset = 0;
			info.range = VK_WHOLE_SIZE;

			auto& desc = writeDescriptors.emplace_back();
			desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc.dstSet = m_descriptorSets[i];
			desc.dstBinding = svkBuffer->getSlot();
			desc.dstArrayElement = 0;
			desc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			desc.descriptorCount = 1; // One binding for each constant buffer / ubo
			desc.pBufferInfo = &info;
		}

		for (auto& sampler : parsed.samplers) {
			VkDescriptorImageInfo info{};
			info.sampler = static_cast<ShaderComponent::SVkSampler*>(sampler.sampler.get())->get();
			
			auto& desc = writeDescriptors.emplace_back();
			desc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc.dstSet = m_descriptorSets[i];
			desc.dstBinding = sampler.res.vkBinding;
			desc.dstArrayElement = 0;
			desc.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			desc.descriptorCount = 1; // One binding for each sampler
			desc.pImageInfo = &info;
		}

	}
	if (!writeDescriptors.empty()) 
		vkUpdateDescriptorSets(m_context->getDevice(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);

}

SVkShader::~SVkShader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);

	vkDeviceWaitIdle(m_context->getDevice());
	vkFreeDescriptorSets(m_context->getDevice(), m_context->getDescriptorPool(), m_descriptorSets.size(), m_descriptorSets.data());
	vkDestroyPipelineLayout(m_context->getDevice(), m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_context->getDevice(), m_descriptorSetLayout, nullptr);
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

	VkShaderModule* shaderModule = SAIL_NEW VkShaderModule;
	if (vkCreateShaderModule(m_context->getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		Logger::Error("Failed to create shader module!");
	}

	return shaderModule;
}

bool SVkShader::onEvent(Event& event) {
	assert(false);
	return true;
}

void SVkShader::prepareToRender(std::vector<Renderer::RenderCommand>& renderCommands) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shader prepareToRender");
	// If the shader wants all textures this frame in an array

	// Find all unique textures and materials
	std::vector<SVkTexture*> uniqueTextures;
	std::vector<Material*> uniqueMaterials;

	unsigned int lastMaterialIndex = 0;
	unsigned int lastTextureIndex = 0;
	for (auto& command : renderCommands) {
		Material* mat = command.material;

		auto it = std::find(uniqueMaterials.begin(), uniqueMaterials.end(), mat);
		if (it == uniqueMaterials.end()) {
			uniqueMaterials.emplace_back(mat);
			command.materialIndex = lastMaterialIndex++;
		} else {
			command.materialIndex = it - uniqueMaterials.begin(); // Get index of the iterator
		}

		unsigned int i = 0;
		for (auto* texture : mat->getTextures()) {
			if (texture) {
				SVkTexture* tex = static_cast<SVkTexture*>(texture);
				auto it = std::find(uniqueTextures.begin(), uniqueTextures.end(), tex);
				if (it == uniqueTextures.end()) {
					uniqueTextures.emplace_back(tex);
					mat->setTextureIndex(i, lastTextureIndex++);
				} else {
					mat->setTextureIndex(i, lastMaterialIndex);
				}

				i++;
			}
		}
	}

	// Find what slot, if any, should be used to bind all textures
	unsigned int textureArrBinding = 0;
	bool shouldBindTextureArr = false;
	for (auto& tex : parser.getParsedData().textures) {
		if (tex.isTexturesArray) {
			textureArrBinding = tex.vkBinding;
			shouldBindTextureArr = true;
			break;
		}
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
	if (materialBuffer && !uniqueMaterials.empty()) {
		// Each shader can only support one material, which means that the size of each is the same
		auto dataSize = uniqueMaterials[0]->getDataSize();

		unsigned int i = 0;
		for (auto& material : uniqueMaterials) {
			materialBuffer->updateData(material->getData(), dataSize, 0, i*dataSize);
			i++;
		}
	}


	VkWriteDescriptorSet descWrite[] = { {}, {} };

	// Create descriptors for textures
	auto imageIndex = m_context->getSwapImageIndex();
	std::vector<VkDescriptorImageInfo> imageInfos;
	if (shouldBindTextureArr) {
		for (auto* texture : uniqueTextures) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (texture->isReadyToUse()) {
				imageInfo.imageView = texture->getView();
			} else {
				imageInfo.imageView = m_missingTexture.getView();
			}
		}

		if (imageInfos.size() > TEXTURE_ARRAY_DESCRIPTOR_COUNT) {
			Logger::Error("Tried to render too many textures (" + std::to_string(imageInfos.size()) + "). Set max/TEXTURE_ARRAY_DESCRIPTOR_COUNT is " + std::to_string(TEXTURE_ARRAY_DESCRIPTOR_COUNT));
		}

		// Fill the unused descriptor slots with the missing texture
		for (unsigned int i = imageInfos.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_missingTexture.getView();
		}

		descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[0].dstSet = m_descriptorSets[imageIndex];
		descWrite[0].dstBinding = textureArrBinding;
		descWrite[0].dstArrayElement = 0;
		descWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		//descWrite[0].descriptorCount = static_cast<uint32_t>(imageInfos.size());
		descWrite[0].descriptorCount = TEXTURE_ARRAY_DESCRIPTOR_COUNT;
		descWrite[0].pImageInfo = imageInfos.data();

		if (imageInfos.empty()) return;
		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[0], 0, nullptr);
	}

	// Create descriptor for materials
	if (materialBuffer)	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = materialBuffer->getBuffer(imageIndex);
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		
		descWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[1].dstSet = m_descriptorSets[imageIndex];
		descWrite[1].dstBinding = materialBinding;
		descWrite[1].dstArrayElement = 0;
		descWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWrite[1].descriptorCount = 1;
		descWrite[1].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[1], 0, nullptr);
	}
}

const VkPipelineLayout& SVkShader::getPipelineLayout() const {
	return m_pipelineLayout;
}

void SVkShader::bind(void* cmdList) const {
	auto imageIndex = m_context->getSwapImageIndex();
	vkCmdBindDescriptorSets(static_cast<VkCommandBuffer>(cmdList), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[imageIndex], 0, nullptr);
	bindInternal(0U, cmdList);
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