#include "pch.h"
#include "HullShader.h"

#include "../../api/Application.h"

HullShader::HullShader(ID3D10Blob* compiledShader) {
	Application::getInstance()->getAPI()->getDevice()->CreateHullShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_shader);
}

HullShader::~HullShader() {
	Memory::safeRelease(m_shader);
}

void HullShader::bind() {
	Application::getInstance()->getAPI()->getDeviceContext()->HSSetShader(m_shader, 0, 0);
}