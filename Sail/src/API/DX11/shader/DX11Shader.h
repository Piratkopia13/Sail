#pragma once

#include "Sail/api/shader/Shader.h"
#include <d3d11.h>

class DX11Shader : public Shader {
public:
	DX11Shader(Shaders::ShaderSettings settings);
	~DX11Shader();

	virtual void bind(void* cmdList) const override;

	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;

	virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;

protected:
	void compile() override;

private:
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11DomainShader* m_ds;
	ID3D11HullShader* m_hs;
	ID3D11GeometryShader* m_gs;

};