#pragma once

#include "../../graphics/shader/component/BindShader.h"
#include <string>

class ShaderCompiler {
public:
	ShaderCompiler() {};
	virtual ~ShaderCompiler() {};

	static void* compile(const std::string& filepath, ShaderComponent::BIND_SHADER shaderType);

protected:

private:

};
