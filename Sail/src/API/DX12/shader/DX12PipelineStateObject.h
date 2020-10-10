#pragma once

#include "../DX12API.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "DXILShaderCompiler.h"

class DX12PipelineStateObject : public PipelineStateObject {
public:
	DX12PipelineStateObject(Shader* shader, unsigned int attributesHash);

	virtual bool bind(void* cmdList) override;
	
private:
	void createGraphicsPipelineState();
	void createComputePipelineState();

private:
	DX12API* m_context;

	static std::unique_ptr<DXILShaderCompiler> m_dxilCompiler; // Class Singleton
	wComPtr<ID3D12PipelineState> m_pipelineState;

};