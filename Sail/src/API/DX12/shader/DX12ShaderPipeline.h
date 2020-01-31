#pragma once

#include "../DX12API.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "DXILShaderCompiler.h"

class DX12ShaderPipeline : public ShaderPipeline {
public:
	DX12ShaderPipeline(const std::string& filename);
	~DX12ShaderPipeline();

	virtual bool bind(void* cmdList, bool forceIfBound = false) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture2D(const std::string& name, Texture* texture, void* cmdList) override;

	void setRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format);
	void setResourceHeapMeshIndex(unsigned int index);

protected:
	virtual void compile() override;
	virtual void finish() override;

private:
	DX12API* m_context;

	std::unordered_map<unsigned int, DXGI_FORMAT> m_rtFormats;
	static std::unique_ptr<DXILShaderCompiler> m_dxilCompiler; // Class Singleton
	wComPtr<ID3D12PipelineState> m_pipelineState;

public:
	void reserve(unsigned int meshIndexMax);
};