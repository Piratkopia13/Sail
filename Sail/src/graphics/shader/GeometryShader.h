#pragma once

#include <d3d11.h>
#include <memory>

class GeometryShader {
public:
	GeometryShader(ID3D10Blob* compiledShader);
	~GeometryShader();

	void bind();

private:
	ID3D11GeometryShader* m_shader;

};