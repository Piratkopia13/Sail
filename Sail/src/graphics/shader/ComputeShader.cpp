#include "pch.h"
#include "ComputeShader.h"

#include "../../api/Application.h"

ComputeShader::ComputeShader(ID3D10Blob* compiledShader) {
	Application::getInstance()->getAPI()->getDevice()->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_shader);
}

ComputeShader::~ComputeShader() {
	Memory::safeRelease(m_shader);
}

void ComputeShader::bind() {
	Application::getInstance()->getAPI()->getDeviceContext()->CSSetShader(m_shader, 0, 0);
}