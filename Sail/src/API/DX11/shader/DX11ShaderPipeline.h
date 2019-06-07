#pragma once

#include <d3d11.h>
#include "Sail/api/shader/ShaderPipeline.h"

class DX11ShaderPipeline : public ShaderPipeline {
public:
	DX11ShaderPipeline(const std::string& filename);
	~DX11ShaderPipeline();

	//virtual void bind();

	// The following static methods are to be implemented in APIs
	virtual void bind(void* cmdList) override;
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) override;
	virtual void setTexture2D(const std::string& name, void* handle) override;
	virtual void compile() override;
	
private:
	ID3D11VertexShader* m_vs;
	ID3D11PixelShader* m_ps;
	ID3D11DomainShader* m_ds;
	ID3D11HullShader* m_hs;
	ID3D11GeometryShader* m_gs;

};