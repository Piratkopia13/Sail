#pragma once

#include <d3d11.h>
#include "Sail/api/shader/PipelineStateObject.h"

class DX11PipelineStateObject : public PipelineStateObject {
public:
	DX11PipelineStateObject(Shader* shader, unsigned int attributesHash);

};