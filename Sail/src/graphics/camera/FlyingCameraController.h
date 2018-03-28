#pragma once

#include "CameraController.h"
#include <SimpleMath.h>

class FlyingCameraController : public CameraController {
public:

	FlyingCameraController(Camera* cam);

	virtual void update(float dt);

private:
	/*Camera* m_cam;
	DirectX::SimpleMath::Quaternion m_rotation;
*/
	float m_yaw, m_pitch, m_roll;
};
