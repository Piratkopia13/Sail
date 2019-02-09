#include "pch.h"
#include "GeometryShader.h"

#include "Sail/Application.h"

GeometryShader::GeometryShader(ID3D10Blob* compiledShader) {
	Application::getInstance()->getAPI()->getDevice()->CreateGeometryShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_shader);
}

GeometryShader::~GeometryShader() {
	Memory::safeRelease(m_shader);
}

void GeometryShader::bind() {
	Application::getInstance()->getAPI()->getDeviceContext()->GSSetShader(m_shader, 0, 0);
}