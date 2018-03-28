#pragma once

#include "Camera.h"
//#include "../api/Application.h"

// Forward declaration
class Application;

class CameraController {
public:

	CameraController(Camera* cam) : m_cam(cam) {};

	virtual void update(float dt) {};

protected:
	void setCameralookAt(const DirectX::SimpleMath::Vector3& pos) {
		m_cam->lookAt(pos);
	}
	void setCameraDirection(const DirectX::SimpleMath::Vector3& direction) {
		m_cam->setDirection(direction);
	}
	void setCameraPosition(const DirectX::SimpleMath::Vector3& pos) {
		m_cam->setPosition(pos);
	}
	const DirectX::SimpleMath::Vector3& getCameraDirection() {
		return m_cam->m_direction;
	}
	const DirectX::SimpleMath::Vector3& getCameraPosition() {
		return m_cam->m_pos;
	}
	const DirectX::SimpleMath::Vector3& getCameraUp() {
		return m_cam->getUp();
	}
	const Camera* getCamera() {
		return m_cam;
	}

private:
	Camera* m_cam;

};
