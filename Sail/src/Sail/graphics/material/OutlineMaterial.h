#pragma once

#include "Material.h"
#include <glm/glm.hpp>

class Shader;

class OutlineMaterial : public Material {
public:
	OutlineMaterial();
	~OutlineMaterial();

	virtual void* getData() override;
	virtual unsigned int getDataSize() const override;
	Shader* getShader(Renderer::Type rendererType) const override;

	void setColor(const glm::vec3& color);
	const glm::vec3& getColor() const;
	void setThickness(float thickness);
	float getThickness() const;

private:
	// Matching shader struct
	struct OutlineSettings {
		glm::vec3 color;
		float thickness;
	};

	OutlineSettings m_settings;

};