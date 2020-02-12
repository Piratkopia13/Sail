#include "pch.h"
#include "DX11PipelineStateObject.h"

PipelineStateObject* PipelineStateObject::Create(Shader* shader, unsigned int attributesHash) {
	return SAIL_NEW DX11PipelineStateObject(shader, attributesHash);
}

DX11PipelineStateObject::DX11PipelineStateObject(Shader* shader, unsigned int attributesHash)
	: PipelineStateObject(shader, attributesHash)
{ }
