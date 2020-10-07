#pragma once

#include "Sail/api/shader/Shader.h"
#include <d3d11.h>

class DX11Shader : public Shader {
public:
	DX11Shader(Shaders::ShaderSettings settings);
	~DX11Shader();

	virtual void bind(void* cmdList) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	void recompile() override;

	void setRenderableTextureUAV(const std::string& name, RenderableTexture* texture);

	void updateDescriptorsAndMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmdList) override;

protected:
	void compile() override;

	bool setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList = nullptr) override;

private:
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11DomainShader* m_ds;
	ID3D11HullShader* m_hs;
	ID3D11GeometryShader* m_gs;
	ID3D11ComputeShader* m_cs;

};