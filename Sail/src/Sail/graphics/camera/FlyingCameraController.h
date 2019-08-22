#pragma once

#include "CameraController.h"
#include <glm/glm.hpp>

class FlyingCameraController : public CameraController {
public:

	FlyingCameraController(Camera* cam);

	void setDirection(const glm::vec3& dir);
	void lookAt(const glm::vec3& pos);

	virtual void update(float dt);

private:
	/*Camera* m_cam;
	DirectX::SimpleMath::Quaternion m_rotation;
*/
	float m_yaw, m_pitch, m_roll;
};
