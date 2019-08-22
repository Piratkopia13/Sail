#include "pch.h"
#include "PixelShader.h"
#include "Sail/Application.h"

PixelShader::PixelShader(ID3D10Blob* compiledShader) {
	Application::getInstance()->getAPI()->getDevice()->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_shader);
}

PixelShader::~PixelShader() {
	Memory::safeRelease(m_shader);
}

void PixelShader::bind() {
	Application::getInstance()->getAPI()->getDeviceContext()->PSSetShader(m_shader, 0, 0);
}