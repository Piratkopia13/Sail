#include "pch.h"
#include "OutlineMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"

OutlineMaterial::OutlineMaterial()
	: Material(Material::OUTLINE)
	, m_color(glm::vec3(0.8f, 0.8f, 0.2f))
	, m_thickness(0.06f)
{ }

OutlineMaterial::~OutlineMaterial() { }

void OutlineMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	shader->setCBufferVar("mat_color", &m_color, sizeof(glm::vec3));
	shader->setCBufferVar("mat_thickness", &m_thickness, sizeof(float));
}

void OutlineMaterial::setColor(const glm::vec3& color) {
	m_color = color;
}

const glm::vec3& OutlineMaterial::getColor() const {
	return m_color;
}

void OutlineMaterial::setThickness(float thickness) {
	m_thickness = thickness;
}

float OutlineMaterial::getThickness() const {
	return m_thickness;
}

