#pragma once

#include <d3d11.h>
#include <memory>
//#include "../../api/Application.h"

class PixelShader {
public:
	PixelShader(ID3D10Blob* compiledShader);
	~PixelShader();

	void bind();

private:
	ID3D11PixelShader* m_shader;

};