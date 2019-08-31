#include "pch.h"
#include "DX12RaytracingRenderer.h"
#include "Sail/Application.h"

DX12RaytracingRenderer::DX12RaytracingRenderer() 
	: m_dxr("potato")
{
	m_context = Application::getInstance()->getAPI<DX12API>();
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() {

}

void DX12RaytracingRenderer::present(RenderableTexture* output) {

}
