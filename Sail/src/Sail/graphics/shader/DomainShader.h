#pragma once

#include <d3d11.h>
#include <memory>

class DomainShader {
public:
	DomainShader(ID3D10Blob* compiledShader);
	~DomainShader();

	void bind();

private:
	ID3D11DomainShader* m_shader;

};