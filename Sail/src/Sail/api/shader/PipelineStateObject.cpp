#include "pch.h"
#include "PipelineStateObject.h"
#include "Sail/Application.h"
#include "Sail/api/shader/Shader.h"
#include <regex>

PipelineStateObject* PipelineStateObject::CurrentlyBoundPSO = nullptr;

PipelineStateObject::PipelineStateObject(Shader* shader, unsigned int attributesHash)
	: settings(shader->getSettings().defaultPSOSettings)
	, shader(shader)
{
	if (!shader->isComputeShader()) {
		inputLayout = std::unique_ptr<InputLayout>(InputLayout::Create());

		// Place each attribute id into the vector
		std::vector<int> meshAttribIDs;
		while (attributesHash > 0) {
			meshAttribIDs.emplace_back(attributesHash % 10);
			attributesHash /= 10;
		}
		attributesHash = shader->getAttributesHash();
		std::vector<int> shaderAttribIDs;
		while (attributesHash > 0) {
			shaderAttribIDs.emplace_back(attributesHash % 10);
			attributesHash /= 10;
		}
		// Parse attributes specified in the shader as well as the mesh being drawn and build the input layout from them

		// Read attribs from the shader and match them up the inputlayout
		// If a shader is missing an attribute defined in the mesh, this attribute will not be bound
		// If a shader specified an attribute not defined in the mesh, this attribute will be bound to always return zero when read (defined in VertexBuffer implementations)
		for (int id : shaderAttribIDs) {
			switch ((InputLayout::InputType)id) {
			case InputLayout::POSITION:
				inputLayout->pushVec3(InputLayout::POSITION, "POSITION", 0, InputLayout::POSITION - 1);
				break;
			case InputLayout::TEXCOORD:
				inputLayout->pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0, InputLayout::TEXCOORD - 1);
				break;
			case InputLayout::NORMAL:
				inputLayout->pushVec3(InputLayout::NORMAL, "NORMAL", 0, InputLayout::NORMAL - 1);
				break;
			case InputLayout::TANGENT:
				inputLayout->pushVec3(InputLayout::TANGENT, "TANGENT", 0, InputLayout::TANGENT - 1);
				break;
			case InputLayout::BITANGENT:
				inputLayout->pushVec3(InputLayout::BITANGENT, "BINORMAL", 0, InputLayout::BITANGENT - 1);
				break;
			}
		}
		inputLayout->create(shader->getVsBlob());
	}
}

PipelineStateObject::~PipelineStateObject() { }

Shader* PipelineStateObject::getShader() const {
	return shader;
}

bool PipelineStateObject::bindInternal(void* cmdList, bool forceIfBound) {
	// Don't bind if already bound
	// This is to cut down on shader state changes
	if (!forceIfBound && CurrentlyBoundPSO == this)
		return false;

	auto* context = Application::getInstance()->getAPI();
	context->setDepthMask(settings.depthMask);
	context->setBlending(settings.blendMode);
	context->setFaceCulling(settings.cullMode);

	shader->bind(cmdList);

	// Input layouts only exist for graphics PSO (not compute)
	if (inputLayout) {
		// Set input layout as active
		inputLayout->bind();
	}

	// Set this shader as bound
	CurrentlyBoundPSO = this;

	return true;
}

// Default bind, override and don't call this one if required by the graphics API
bool PipelineStateObject::bind(void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	return bindInternal(cmdList, false);
}

void PipelineStateObject::unbind(){
	CurrentlyBoundPSO = nullptr;
}
