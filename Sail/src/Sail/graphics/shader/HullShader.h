#pragma once

#include <d3d11.h>
#include <memory>

class HullShader {
public:
	HullShader(ID3D10Blob* compiledShader);
	~HullShader();

	void bind();

private:
	ID3D11HullShader* m_shader;

};