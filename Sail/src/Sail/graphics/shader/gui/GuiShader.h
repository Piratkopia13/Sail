#pragma once

#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"

class GuiShader : public Shader {
public:
	GuiShader();
	~GuiShader();

	virtual void bind() override;

private:

};
