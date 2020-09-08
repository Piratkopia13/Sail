#include "pch.h"
#include "SVkShader.h"
#include "../SVkAPI.h"
#include "Sail/Application.h"
//#include "VkConstantBuffer.h"
//#include "../resources/DescriptorHeap.h"
#include "../resources/VkTexture.h"
#include "SVkConstantBuffer.h"
//#include "../resources/VkRenderableTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW SVkShader(settings);
	else
		return new (allocAddr) SVkShader(settings);
}

SVkShader::SVkShader(Shaders::ShaderSettings settings)
	: Shader(settings)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<SVkAPI>();

	compile();

	// Create the pipeline layout from parsed data
	// Uniform buffer objects in vulkan are used the same way as constant buffers in dx12
	auto& cBuffers = parser.getParsedData().cBuffers;
	std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings;
	for (auto& cbuffer : cBuffers) {
		VkDescriptorSetLayoutBinding b;
		b.binding = static_cast<uint32_t>(cbuffer.cBuffer->getSlot());
		b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		b.descriptorCount = 1;
		b.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // TODO: make this more specific(?)
		b.pImmutableSamplers = nullptr; // Optional
		uboLayoutBindings.emplace_back(b);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(uboLayoutBindings.size());
	layoutInfo.pBindings = uboLayoutBindings.data();

	if (vkCreateDescriptorSetLayout(m_context->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
		Logger::Error("Failed to create descriptor set layout!");
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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

	VkShaderModule* shaderModule = new VkShaderModule;
	if (vkCreateShaderModule(m_context->getDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		Logger::Error("Failed to create shader module!");
	}

	return shaderModule;
}

bool SVkShader::onEvent(Event& event) {
	assert(false);
	return true;
}

const VkPipelineLayout& SVkShader::getPipelineLayout() const {
	return m_pipelineLayout;
}

void SVkShader::bind(void* cmdList, uint32_t frameIndex) const {
	vkCmdBindDescriptorSets(*static_cast<VkCommandBuffer*>(cmdList), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[frameIndex], 0, nullptr);
	bindInternal(0U, cmdList);
}

bool SVkShader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	assert(false);
	return true;
}

void SVkShader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	assert(false);
}

void SVkShader::setCBufferVar(const std::string& name, const void* data, unsigned int size) {
	setCBufferVarInternal(name, data, size, 0U);
}

bool SVkShader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size) {
	return trySetCBufferVarInternal(name, data, size, 0U);
}