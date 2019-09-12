#pragma once

#include "../DX12API.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "DXILShaderCompiler.h"

class DX12ShaderPipeline : public ShaderPipeline {
public:
	DX12ShaderPipeline(const std::string& filename);
	~DX12ShaderPipeline();

	virtual void bind(void* cmdList) override;
	// Only used by compute shaders
	virtual void dispatch(unsigned int threadGroupCountX, unsigned int threadGroupCountY, unsigned int threadGroupCountZ, void* cmdList = nullptr) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture2D(const std::string& name, Texture* texture, void* cmdList) override;

	void setResourceHeapMeshIndex(unsigned int index);


private:
	void createGraphicsPipelineState();
	void createComputePipelineState();

protected:
	virtual void compile() override;
	virtual void finish() override;

private:

	DX12API* m_context;

	static std::unique_ptr<DXILShaderCompiler> m_dxilCompiler; // Class Singleton
	wComPtr<ID3D12PipelineState> m_pipelineState;

};