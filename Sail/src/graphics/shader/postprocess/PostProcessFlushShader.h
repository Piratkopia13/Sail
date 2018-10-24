#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../../../api/Application.h"

class PostProcessFlushShader : public ShaderSet {
public:
	PostProcessFlushShader();
	~PostProcessFlushShader();

	void bind() override;

};
