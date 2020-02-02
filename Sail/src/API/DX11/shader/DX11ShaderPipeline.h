#pragma once

#include <d3d11.h>
#include "Sail/api/shader/ShaderPipeline.h"

class DX11ShaderPipeline : public ShaderPipeline {
public:
	DX11ShaderPipeline(const std::string& filename);
	~DX11ShaderPipeline();

	//virtual void bind();

	// The following static methods are to be implemented in APIs
	virtual bool bind(void* cmdList) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture(const std::string& name, Texture* texture, void* cmdList) override;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) override;
	virtual void compile() override;
	


private:
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11DomainShader* m_ds;
	ID3D11HullShader* m_hs;
	ID3D11GeometryShader* m_gs;

};