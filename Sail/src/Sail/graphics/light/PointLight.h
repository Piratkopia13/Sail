#pragma once
#include <d3d11.h>
#include <SimpleMath.h>

class PointLight {
public:
	struct Attenuation {
		float constant;
		float linear;
		float quadratic;
	};
public:
	PointLight() : m_color(DirectX::SimpleMath::Vector3::Zero), m_position(DirectX::SimpleMath::Vector3::Zero), m_attenuation({ 1.f, 1.f, 1.f }) { }
	void setColor(const DirectX::SimpleMath::Vector3& color) { m_color = color; }
	const DirectX::SimpleMath::Vector3& getColor() const { return m_color; }
	void setPosition(const DirectX::SimpleMath::Vector3& position) { m_position = position; }
	const DirectX::SimpleMath::Vector3& getPosition() const { return m_position; }
	void setAttenuation(float constant, float linear, float quadratic) {
		m_attenuation.constant = constant;
		m_attenuation.linear = linear;
		m_attenuation.quadratic = quadratic;
		calculateRadius();
	}
	const Attenuation& getAttenuation() const { return m_attenuation; }
	float getRadius() const { return m_radius; }
private:
	void calculateRadius() {
		// Derived from attenuation formula used in light shader
		m_radius = (-m_attenuation.linear + std::sqrt(std::pow(m_attenuation.linear, 2.f) - 4.f * m_attenuation.quadratic*(m_attenuation.constant - 100))) / (2 * m_attenuation.quadratic);
	}
private:
	DirectX::SimpleMath::Vector3 m_color;
	DirectX::SimpleMath::Vector3 m_position;
	Attenuation m_attenuation;
	float m_radius;
};