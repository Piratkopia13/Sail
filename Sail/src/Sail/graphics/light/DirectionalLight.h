#pragma once
#include <d3d11.h>
#include <SimpleMath.h>

class DirectionalLight {
public:
	DirectionalLight(const DirectX::SimpleMath::Vector3& color = DirectX::SimpleMath::Vector3(.8f), 
		const DirectX::SimpleMath::Vector3& dir = DirectX::SimpleMath::Vector3(0.f, -1.f, 0.f)) 
		: m_color(color), m_direction(dir) 
	{ }

	const DirectX::SimpleMath::Vector3& getColor() const {
		return m_color;
	}
	void setColor(const DirectX::SimpleMath::Vector3& color) {
		m_color = color;
	}
	const DirectX::SimpleMath::Vector3& getDirection() const {
		return m_direction;
	}
	void setDirection(const DirectX::SimpleMath::Vector3& dir) {
		m_direction = dir;
	}

private:
	DirectX::SimpleMath::Vector3 m_color;
	DirectX::SimpleMath::Vector3 m_direction;
};
