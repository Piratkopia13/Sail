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
	virtual void setWireframe(bool wireframe);
	virtual void setCullMode(GraphicsAPI::Culling cullMode);

protected:
	void finish();
protected:
	// This is a raw pointer and not a smart pointer because reload() requires new (*) T() functionality
	ShaderPipeline* shaderPipeline;
private:
	bool m_finished;

};