#pragma once

#include "../DX12API.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "DXILShaderCompiler.h"

class DX12ShaderPipeline : public ShaderPipeline {
public:
	DX12ShaderPipeline(const std::string& filename);
	~DX12ShaderPipeline();

	virtual void bind(void* cmdList) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture2D(const std::string& name, void* handle) override;

	void setResourceHeapMeshIndex(unsigned int index);

protected:
	virtual void compile() override;
	virtual void finish() override;

private:
	// Index used when updating and binding cbuffers
	// The index specifies which offset it should write/read from in the resource heap
	//unsigned int m_resourceHeapMeshIndex;

	static std::unique_ptr<DXILShaderCompiler> m_dxilCompiler; // Class Singleton
	wComPtr<ID3D12PipelineState> m_pipelineState;

};