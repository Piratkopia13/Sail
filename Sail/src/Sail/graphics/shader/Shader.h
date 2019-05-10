#pragma once

#include "Sail/api/shader/ShaderPipeline.h"
#include <memory>
#include <string>

class Shader {
public:
	
	Shader(const std::string& filename);
	virtual ~Shader();

	ShaderPipeline* getPipeline();

	virtual void bind();
	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};

protected:
	std::unique_ptr<ShaderPipeline> shaderPipeline;

};