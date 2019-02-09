#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "Sail/Application.h"
#include "../../geometry/Material.h"

class DeferredGeometryShader : public ShaderSet {
public:
	DeferredGeometryShader();
	~DeferredGeometryShader();

	void bind() override;
};
