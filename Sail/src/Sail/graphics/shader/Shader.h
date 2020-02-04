#pragma once

#include "Sail/api/shader/ShaderPipeline.h"
#include <memory>
#include <string>

class Shader {
public:
	struct ComputeSettings {
		float threadGroupXScale = 1.0f;
		float threadGroupYScale = 1.0f;
		float threadGroupZScale = 1.0f;
	};
	
	Shader(const std::string& filename);
	virtual ~Shader();

	ShaderPipeline* getPipeline();
	Material::Type getMaterialType() const;

	virtual void bind();
	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};
	virtual void setWireframe(bool wireframe);
	virtual void setCullMode(GraphicsAPI::Culling cullMode);

	// Compute specific
	virtual const ComputeSettings* getComputeSettings() const { return nullptr; };
protected:
	void setMaterialType(Material::Type type);
	void finish();
protected:
	// This is a raw pointer and not a smart pointer because reload() requires new (*) T() functionality
	ShaderPipeline* shaderPipeline;
private:
	bool m_finished;
	Material::Type m_materialType;

};