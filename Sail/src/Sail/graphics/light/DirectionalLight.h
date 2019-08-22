#pragma once
#include <d3d11.h>
#include <glm/glm.hpp>

class DirectionalLight {
public:
	DirectionalLight(const glm::vec3& color = glm::vec3(.8f), 
		const glm::vec3& dir = glm::vec3(0.f, -1.f, 0.f)) 
		: m_color(color), m_direction(dir) 
	{ }

	const glm::vec3& getColor() const {
		return m_color;
	}
	void setColor(const glm::vec3& color) {
		m_color = color;
	}
	const glm::vec3& getDirection() const {
		return m_direction;
	}
	void setDirection(const glm::vec3& dir) {
		m_direction = dir;
	}

private:
	glm::vec3 m_color;
	glm::vec3 m_direction;
};
