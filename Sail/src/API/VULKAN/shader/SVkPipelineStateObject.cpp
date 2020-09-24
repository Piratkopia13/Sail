#include "pch.h"
#include "SVkPipelineStateObject.h"
#include "Sail/Application.h"
#include "SVkInputLayout.h"
#include "SVkShader.h"

PipelineStateObject* PipelineStateObject::Create(Shader* shader, unsigned int attributesHash) {
	return SAIL_NEW SVkPipelineStateObject(shader, attributesHash);
}

SVkPipelineStateObject::SVkPipelineStateObject(Shader* shader, unsigned int attributesHash)
	: PipelineStateObject(shader, attributesHash)
	, m_pipeline()
{
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	
	// Create a compute pipeline state if it has a compute shader
	if (shader->isComputeShader()) {
		createComputePipelineState();
	} else {
		createGraphicsPipelineState();
	}

	m_vkShader = static_cast<SVkShader*>(shader);
	// Create the descriptor set according to the layout as defined in the shader
	m_vkShader->createDescriptorSet(m_descriptorSets);
}

SVkPipelineStateObject::~SVkPipelineStateObject() {
	vkDestroyPipeline(m_context->getDevice(), m_pipeline, nullptr);
	vkFreeDescriptorSets(m_context->getDevice(), m_context->getDescriptorPool(), m_descriptorSets.size(), m_descriptorSets.data());
}

bool SVkPipelineStateObject::bind(void* cmdList) {
	if (!m_pipeline)
		Logger::Error("Tried to bind pipeline state before it has been created!");

	// TODO: This returns false if pipeline is already bound, maybe do something with that
	bindInternal(cmdList, true);

	vkCmdBindPipeline(static_cast<VkCommandBuffer>(cmdList), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	vkCmdBindDescriptorSets(static_cast<VkCommandBuffer>(cmdList), VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkShader->getPipelineLayout(), 0, 1, &getDescriptorSet(), 0, nullptr);

	return true;
}

const VkDescriptorSet& SVkPipelineStateObject::getDescriptorSet() const {
	auto imageIndex = m_context->getSwapImageIndex();
	return m_descriptorSets[imageIndex];
}

void SVkPipelineStateObject::createGraphicsPipelineState() {

	VkShaderModule* vsModule = static_cast<VkShaderModule*>(shader->getVsBlob());
	VkShaderModule* psModule = static_cast<VkShaderModule*>(shader->getPsBlob());
	VkShaderModule* gsModule = static_cast<VkShaderModule*>(shader->getGsBlob());
	VkShaderModule* dsModule = static_cast<VkShaderModule*>(shader->getDsBlob());
	VkShaderModule* hsModule = static_cast<VkShaderModule*>(shader->getHsBlob());
	
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	auto createShaderStageInfo = [](VkShaderStageFlagBits stage, VkShaderModule* shaderModule, const char* entrypoint) {
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = *shaderModule;
		shaderStageInfo.pName = entrypoint;
		
		return shaderStageInfo;
	};

	if (vsModule) {
		shaderStages.emplace_back(createShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, vsModule, "VSMain"));
	}
	if (psModule) {
		shaderStages.emplace_back(createShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, psModule, "PSMain"));
	}
	if (gsModule) {
		shaderStages.emplace_back(createShaderStageInfo(VK_SHADER_STAGE_GEOMETRY_BIT, gsModule, "GSMain"));
	}
	if (dsModule) {
		shaderStages.emplace_back(createShaderStageInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, dsModule, "DSMain"));
	}
	if (hsModule) {
		shaderStages.emplace_back(createShaderStageInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, hsModule, "HSMain"));
	}

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport and scissors
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr; // Set dynamically during rendering
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr; // Set dynamically during rendering

	// Rasterizer state
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
		
	if (settings.wireframe) {
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	} else {
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
	rasterizer.lineWidth = 1.0f;

	if (settings.cullMode == GraphicsAPI::Culling::NO_CULLING) {
		rasterizer.cullMode = VK_CULL_MODE_NONE;
	} else if (settings.cullMode == GraphicsAPI::Culling::FRONTFACE) {
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
	} else if (settings.cullMode == GraphicsAPI::Culling::BACKFACE) {
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	}

	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = (m_context->getSampleCount() > 1); // TODO: this is quite expensive, consider disabling
	multisampling.rasterizationSamples = m_context->getSampleCount();
	multisampling.minSampleShading = 0.2f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = (settings.depthMask != GraphicsAPI::BUFFER_DISABLED);
	depthStencil.depthWriteEnable = (settings.depthMask != GraphicsAPI::WRITE_MASK);
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	if (settings.blendMode == GraphicsAPI::ALPHA) {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	} else if (settings.blendMode == GraphicsAPI::ADDITIVE) {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	// Use the same blending for all render targets
	// TODO: make blending configurable for each RT
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	for (unsigned int i = 0; i < settings.numRenderTargets; i++) {
		colorBlendAttachments.emplace_back(colorBlendAttachment);
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = settings.numRenderTargets;
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Set up dynamic states to allow window resizing without recreating pipelines
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = ARRAYSIZE(dynamicStates);
	dynamicState.pDynamicStates = dynamicStates;

	// Finally, create the graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &static_cast<SVkInputLayout*>(inputLayout.get())->getCreateInfo();
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState; // These are dynamic
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = static_cast<SVkShader*>(shader)->getPipelineLayout();
	pipelineInfo.renderPass = m_context->getRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(m_context->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
		Logger::Error("Failed to create graphics pipeline!");
	}

}

void SVkPipelineStateObject::createComputePipelineState() {
	assert(false);
}