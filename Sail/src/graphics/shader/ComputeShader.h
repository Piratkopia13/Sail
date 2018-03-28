#pragma once

#include <d3d11.h>
#include <memory>

class ComputeShader {
public:
	ComputeShader(ID3D10Blob* compiledShader);
	~ComputeShader();

	void bind();

private:
	ID3D11ComputeShader* m_shader;

};