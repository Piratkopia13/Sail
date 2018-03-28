#include "VertexShader.h"
#include "../../api/Application.h"

VertexShader::VertexShader(ID3D10Blob* compiledShader) {
	Application::getInstance()->getAPI()->getDevice()->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_shader);
}

VertexShader::~VertexShader() {
	Memory::safeRelease(m_shader);
}

void VertexShader::bind() {
	Application::getInstance()->getAPI()->getDeviceContext()->VSSetShader(m_shader, 0, 0);
}
