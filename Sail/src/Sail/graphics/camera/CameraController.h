#pragma once

#include "Camera.h"
//#include "../api/Application.h"

// Forward declaration
class Application;

class CameraController {
public:

	CameraController(Camera* cam) : m_cam(cam) {};

	virtual void update(float dt) {};

//protected:
	void setCameralookAt(const glm::vec3& pos) {
		m_cam->lookAt(pos);
	}
	void setCameraDirection(const glm::vec3& direction) {
		m_cam->setDirection(direction);
	}
	void setCameraPosition(const glm::vec3& pos) {
		m_cam->setPosition(pos);
	}
	const glm::vec3& getCameraDirection() {
		return m_cam->m_direction;
	}
	const glm::vec3& getCameraPosition() {
		return m_cam->m_pos;
	}
	const glm::vec3& getCameraUp() {
		return m_cam->getUp();
	}
	const Camera* getCamera() {
		return m_cam;
	}

private:
	Camera* m_cam;

};
