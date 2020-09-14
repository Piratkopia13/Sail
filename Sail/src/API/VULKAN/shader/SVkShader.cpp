#include "pch.h"
#include "SVkShader.h"
#include "../SVkAPI.h"
#include "Sail/Application.h"
#include "SVkConstantBuffer.h"
#include "../SVkUtils.h"
#include "../resources/SVkTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW SVkShader(settings);
	else
		return new (allocAddr) SVkShader(settings);
}

SVkShader::SVkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
	, m_tempSampler(Texture::WRAP, Texture::ANISOTROPIC, ShaderComponent::PS, 0)
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
		b.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//b.descriptorCount = (texture.arraySize == -1) ? 128 : texture.arraySize; // an array size of -1 means it is sizeless in the shader
		b.descriptorCount = (texture.arraySize == -1) ? 2 : texture.arraySize; // an array size of -1 means it is sizeless in the shader
		b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		b.pImmutableSamplers = nullptr; // Optional
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_context->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		Logger::Error("Failed to create descriptor set layout!");
	}

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

	if (vkCreatePipelineLayout(m_context->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
		Logger::Error("Failed to create pipeline layout!");
	}

	// Create one descriptor set per swap image

	auto numBuffers = m_context->getNumSwapChainImages();
	std::vector<VkDescriptorSetLayout> layouts(numBuffers, m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_context->getDescriptorPool();
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numBuffers);
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(numBuffers);
	if (vkAllocateDescriptorSets(m_context->getDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
		Logger::Error("Failed to allocate descriptor sets!");
	}

	// Configure the descriptor sets
	// TODO: add support for dynamic uniform buffers (shared between multiple instances)
	for (size_t i = 0; i < numBuffers; i++) {
		std::vector<VkDescriptorBufferInfo> bufferInfos;
		for (auto& buffer : cBuffers) {
			auto* svkBuffer = static_cast<ShaderComponent::SVkConstantBuffer*>(buffer.cBuffer.get());
			
			VkDescriptorBufferInfo info{};
			info.buffer = svkBuffer->getBuffer(i);
			info.offset = 0;
			info.range = VK_WHOLE_SIZE;

			bufferInfos.emplace_back(info);
		}

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = bufferInfos.size(); // One binding for each constant buffer / ubo
		descriptorWrite.pBufferInfo = bufferInfos.data();
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

}

SVkShader::~SVkShader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);

	vkDeviceWaitIdle(m_context->getDevice());
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
	/*auto shaderCode = Utils::readFileBinary("res/shaders/vulkan/vert.spv");
	if (shaderType == ShaderComponent::BIND_SHADER::PS)
		shaderCode = Utils::readFileBinary("res/shaders/vulkan/frag.spv");*/

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

void SVkShader::updateDescriptorSet(void* cmdList) {
	if (m_imageInfos.empty()) return;

	auto imageIndex = m_context->getSwapImageIndex();
	VkWriteDescriptorSet descWrite{};
	descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrite.dstSet = m_descriptorSets[imageIndex];
	descWrite.dstBinding = 5; // Used for combined image samplers
	descWrite.dstArrayElement = 0;
	descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//descWrite.descriptorCount = static_cast<uint32_t>(m_imageInfos.size());
	descWrite.descriptorCount = 2;
	descWrite.pImageInfo = m_imageInfos.data();
	vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite, 0, nullptr);

	m_imageInfos.clear();
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
	if (!texture) return false; // No texture bound to this slot

	auto* vkTexture = static_cast<SVkTexture*>(texture);
	
	for (unsigned int i = 0; i < 2; i++) {
		auto& imageInfo = m_imageInfos.emplace_back();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = m_tempSampler.get();
		if (vkTexture->isReadyToUse() && i == 0) {
			imageInfo.imageView = vkTexture->getView();
		} else {
			imageInfo.imageView = m_missingTexture.getView();
		}
	}

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