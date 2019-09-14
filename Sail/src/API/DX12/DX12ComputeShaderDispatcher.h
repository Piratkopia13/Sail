#pragma once

#include "Sail/api/ComputeShaderDispatcher.h"
#include "DX12API.h"

class DX12ComputeShaderDispatcher : public ComputeShaderDispatcher {
public:
	DX12ComputeShaderDispatcher(Shader& computeShader);
	~DX12ComputeShaderDispatcher();

	virtual void dispatch(void* cmdList) override;
	virtual Shader::ComputeShaderOutput& getOutput() override;

private:
	DX12API* m_context;

};