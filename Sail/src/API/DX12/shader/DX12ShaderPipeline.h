#pragma once

#include "../DX12API.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "DXILShaderCompiler.h"
#include "Sail/events/Events.h"

class DX12ShaderPipeline : public ShaderPipeline, public IEventListener {
public:
	DX12ShaderPipeline(const std::string& filename);
	~DX12ShaderPipeline();

	virtual bool bind(void* cmdList) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture(const std::string& name, Texture* texture, void* cmdList) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) override;

	void setRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format);
	
	// Call this after each mesh/instance
	// Internally updates meshIndex used to place multiple instances in a single cbuffer
	void instanceFinished();
	void reserve(unsigned int meshIndexMax);

	bool onEvent(Event& event) override;

	void setCBufferVar(const std::string& name, const void* data, UINT size) override;
	bool trySetCBufferVar(const std::string& name, const void* data, UINT size) override;

protected:
	virtual void compile() override;
	virtual void finish() override;

private:
	unsigned int getMeshIndex();
	void createGraphicsPipelineState();
	void createComputePipelineState();

private:
	DX12API* m_context;

	std::atomic_uint m_meshIndex[2];
	std::unordered_map<unsigned int, DXGI_FORMAT> m_rtFormats;
	static std::unique_ptr<DXILShaderCompiler> m_dxilCompiler; // Class Singleton
	wComPtr<ID3D12PipelineState> m_pipelineState;

};