#pragma once

#include "Material.h"
#include <glm/glm.hpp>

class Shader;

class OutlineMaterial : public Material {
public:
	OutlineMaterial();
	~OutlineMaterial();

	virtual void bind(Shader* shader, Environment* environment, void* cmdList = nullptr) override;
	Shader* getShader(Renderer::Type rendererType) const override;

	void setColor(const glm::vec3& color);
	const glm::vec3& getColor() const;
	void setThickness(float thickness);
	float getThickness() const;

private:
	glm::vec3 m_color;
	float m_thickness;

};