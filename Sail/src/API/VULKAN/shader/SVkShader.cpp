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
		b.descriptorType = (texture.isWritable) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		b.descriptorCount = (texture.isTexturesArray || texture.isTextureCubesArray) ? TEXTURE_ARRAY_DESCRIPTOR_COUNT : (texture.arraySize == -1) ? 64 : texture.arraySize; // an array size of -1 means it is sizeless in the shader, set some max number
		b.stageFlags = (parsed.hasCS) ? VK_SHADER_STAGE_COMPUTE_BIT : VK_SHADER_STAGE_FRAGMENT_BIT; // TODO: add support for the other stages
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
	for (auto& pc : parser.getParsedData().constants) {
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
		cmd << " -D _SAIL_VK=1"; // Sets a define to allow shaders to change if compiling for vulkan
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

void SVkShader::updateDescriptorsAndMaterialIndices(std::vector<Renderer::RenderCommand>& renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmd) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shader updateDescriptorsAndMaterialIndices");
	
	DescriptorUpdateInfo updateInfo = {};
	getDescriptorUpdateInfoAndUpdateMaterialIndices(renderCommands, environment, &updateInfo);

	auto svkPso = static_cast<const SVkPipelineStateObject*>(pso);
	VkWriteDescriptorSet descWrite[] = { {}, {}, {} };

	auto swapIndex = m_context->getSwapIndex();
	// Create descriptors for the 2D texture array
	if (updateInfo.bindTextureArray) {
		std::vector<VkDescriptorImageInfo> imageInfos;
		auto imageLayout = (updateInfo.textureArrayIsWritable) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Set according to vk spec
		auto descType = (updateInfo.textureArrayIsWritable) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		for (auto* texture : updateInfo.uniqueTextures) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = imageLayout;
			if (texture->isReadyToUse()) {
				imageInfo.imageView = static_cast<SVkTexture*>(texture)->getView();
			} else {
				imageInfo.imageView = m_missingTexture.getView();
			}
		}
		for (auto* rendTexture : updateInfo.uniqueRenderableTextures) {
			// Renderable textures are always ready to use
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = imageLayout;
			imageInfo.imageView = static_cast<SVkRenderableTexture*>(rendTexture)->getView();
		}

		// Fill the unused descriptor slots with the missing texture
		for (unsigned int i = imageInfos.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = imageLayout;
			imageInfo.imageView = m_missingTexture.getView();
			if (descType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
				// We can not use the default missing texture since it has compression not compatible with IMAGE_STORAGE_BIT
				// Use the first available texture view instead, and fail if there is none
				assert(!updateInfo.uniqueTextures.empty());
				imageInfo.imageView = static_cast<SVkTexture*>(updateInfo.uniqueTextures[0])->getView();
			}
		}

		descWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[0].dstSet = svkPso->getDescriptorSet();
		descWrite[0].dstBinding = updateInfo.textureArrayBinding;
		descWrite[0].dstArrayElement = 0;
		descWrite[0].descriptorType = descType;
		//descWrite[0].descriptorCount = static_cast<uint32_t>(imageInfos.size());
		descWrite[0].descriptorCount = TEXTURE_ARRAY_DESCRIPTOR_COUNT;
		descWrite[0].pImageInfo = imageInfos.data();

		if (imageInfos.empty()) return;
		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[0], 0, nullptr);
	}

	// Create descriptors for the texture cube array
	if (updateInfo.bindTextureCubeArray) {
		std::vector<VkDescriptorImageInfo> imageInfos;
		for (auto* texture : updateInfo.uniqueTextureCubes) {
			auto& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (texture->isReadyToUse()) {
				imageInfo.imageView = static_cast<SVkTexture*>(texture)->getView();
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
		descWrite[1].dstSet = svkPso->getDescriptorSet();
		descWrite[1].dstBinding = updateInfo.textureCubeArrayBinding;
		descWrite[1].dstArrayElement = 0;
		descWrite[1].descriptorType = (updateInfo.textureCubeArrayIsWritable) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descWrite[1].descriptorCount = TEXTURE_ARRAY_DESCRIPTOR_COUNT;
		descWrite[1].pImageInfo = imageInfos.data();

		if (imageInfos.empty()) return;
		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[1], 0, nullptr);
	}

	// Create descriptor for materials
	if (updateInfo.materialBuffer) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = static_cast<ShaderComponent::SVkConstantBuffer*>(updateInfo.materialBuffer)->getBuffer(swapIndex);
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		descWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrite[2].dstSet = svkPso->getDescriptorSet();
		descWrite[2].dstBinding = updateInfo.materialBuffer->getSlot();
		descWrite[2].dstArrayElement = 0;
		descWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descWrite[2].descriptorCount = 1;
		descWrite[2].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_context->getDevice(), 1, &descWrite[2], 0, nullptr);
	}
}

void SVkShader::updateDescriptors(const Descriptors& descriptors, const SVkPipelineStateObject* pso) {
	auto& parsed = parser.getParsedData();
	std::vector<VkWriteDescriptorSet> sets;

	// Handle Image Descriptors
	{
		auto addImageDescSet = [&sets, &pso](const ShaderParser::ShaderResource& res, const std::vector<VkDescriptorImageInfo>& imageInfos) {
			auto& s = sets.emplace_back();
			s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			s.dstSet = pso->getDescriptorSet();
			s.dstBinding = res.vkBinding;
			s.dstArrayElement = 0;
			s.descriptorType = (res.isWritable) ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			s.descriptorCount = static_cast<uint32_t>(imageInfos.size());
			s.pImageInfo = imageInfos.data();
		};
		for (auto& desc : descriptors.images) {
			// Handle desc if it's a texture
			{
				auto& it = std::find_if(parsed.textures.begin(), parsed.textures.end(),
					[&desc](const ShaderParser::ShaderResource& x) { return x.name == desc.name; });
				if (it != parsed.textures.end()) {
					addImageDescSet(*it, desc.infos);
					continue;
				}
			}
			// Handle desc if it's a renderableTexture
			{
				auto& it = std::find_if(parsed.renderableTextures.begin(), parsed.renderableTextures.end(),
					[&desc](const ShaderParser::ShaderRenderableTexture& x) { return x.res.name == desc.name; });
				if (it != parsed.renderableTextures.end()) {
					auto& s = sets.emplace_back();
					addImageDescSet(it->res, desc.infos);
					continue;
				} else {
					Logger::Error("Descriptor name not found in shader");
				}
			}
		}
	}

	// Handle Buffer Descriptors
	{
		for (auto& desc : descriptors.buffers) {
			auto& it = std::find_if(parsed.cBuffers.begin(), parsed.cBuffers.end(),
				[&desc](const ShaderParser::ShaderCBuffer& x) { return x.name == desc.name; });
			if (it != parsed.cBuffers.end()) {
				auto& s = sets.emplace_back();
				s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				s.dstSet = pso->getDescriptorSet();
				s.dstBinding = it->cBuffer->getSlot();
				s.dstArrayElement = 0;
				s.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				s.descriptorCount = static_cast<uint32_t>(desc.infos.size());
				s.pBufferInfo = desc.infos.data();
				continue;
			} else {
				Logger::Error("Descriptor name not found in shader");
			}
		}
	}

	vkUpdateDescriptorSets(m_context->getDevice(), (uint32_t)sets.size(), sets.data(), 0, nullptr);
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
	bindInternal(cmdList);
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

bool SVkShader::setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList) {
	vkCmdPushConstants(static_cast<VkCommandBuffer>(cmdList), m_pipelineLayout, SVkUtils::ConvertShaderBindingToStageFlags(bindShader), byteOffset, size, data);
	return true;
}

void SVkShader::createDescriptorSet(std::vector<VkDescriptorSet>& outDescriptorSet) const {
	// Create one descriptor set per swap image

	auto& parsed = parser.getParsedData();

	auto numBuffers = m_context->getNumSwapBuffers();
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
			if (buffer.isMaterialArray) continue;
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
