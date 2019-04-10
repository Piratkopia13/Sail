#pragma once

#include <d3d11.h>
#include "../ShaderSet.h"
#include "Sail/Application.h"

class PostProcessFlushShader : public ShaderSet {
public:
	PostProcessFlushShader();
	~PostProcessFlushShader();

	void bind() override;

};
