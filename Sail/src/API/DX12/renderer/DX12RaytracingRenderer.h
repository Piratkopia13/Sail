#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "../DXR/DXRBase.h"

class DX12RaytracingRenderer : public Renderer {
public:
	DX12RaytracingRenderer();
	~DX12RaytracingRenderer();

	void present(RenderableTexture* output = nullptr) override;

private:
	DX12API* m_context;
	DX12API::Command m_command;
	DXRBase m_dxr;

};