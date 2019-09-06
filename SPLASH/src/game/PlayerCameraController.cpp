#include "PlayerCameraController.h"
#include <algorithm>

using namespace DirectX;
using namespace DirectX::SimpleMath;

PlayerCameraController::PlayerCameraController(Camera* cam, const DirectX::SimpleMath::Vector2* mapSize)
	: CameraController(cam)
	, m_lockToMap(mapSize != nullptr)
	, m_extraZCurrent(0.f)
	, m_extraZTarget(0.f)
	, m_cameraYOffset(1.0f)
	, m_cameraZOffset(0.0f)
	, m_followSpeed(5.0f)
	, m_moveSpeed(2)
	, m_position(DirectX::SimpleMath::Vector3(0, 0, 0))
	, m_target(DirectX::SimpleMath::Vector3(0, 0, 0))
	, m_back(Vector3(0, 0, -1))
	, m_up(Vector3(0, 1, 0))
	, m_moving(true)
	, m_useExtraZ(true)
	, m_timeZoomedIn(0.f)
{

	if (m_lockToMap)
		m_mapSize = Vector2(*mapSize);


	for (int i = 0; i < 4; i++)
		m_targets[i] = nullptr;
}

void PlayerCameraController::update(float dt) {

	updatePosition(dt);

	setCameraPosition(m_position + m_back * (m_cameraZOffset + m_extraZCurrent) + m_up * m_cameraYOffset);
	setCameralookAt(m_target);
}

void PlayerCameraController::setPosition(Vector3 pos)
{
	m_position = pos;
}

void PlayerCameraController::setTarget(Vector3 pos)
{
	m_target = pos;
}

void PlayerCameraController::setOffset(Vector3 offset)
{
	m_cameraZOffset = offset.z;
	m_cameraYOffset = offset.y;
}

void PlayerCameraController::setMoveSpeed(float speed)
{
	m_moveSpeed = speed;
}

void PlayerCameraController::setFollowSpeed(float speed)
{
	m_followSpeed = speed;
}

void PlayerCameraController::setTargets(Object* focusObject1, Object* focusObject2, Object* focusObject3, Object* focusObject4)
{
	m_targets[0] = focusObject1;
	m_targets[1] = focusObject2;
	m_targets[2] = focusObject3;
	m_targets[3] = focusObject4;
}

Vector3 PlayerCameraController::getPosition()
{
	return m_position;
}

Vector3 PlayerCameraController::getTarget()
{
	return m_target;
}

void PlayerCameraController::setMoving(bool moving)
{
	m_moving = moving;
}

void PlayerCameraController::setUseExtraZ(bool use) {
	m_useExtraZ = use;
}

void PlayerCameraController::updatePosition(float dt)
{
	Vector3 newTarget(0, 0, 0);
	int nr = 0;
	float maxXDst = 0;
	float maxYDst = 0;

	if (m_targets[0] != nullptr) {
		for (int i = 0; i < 4; i++) {
			if (m_targets[i]) {
				newTarget += m_targets[i]->getTransform().getTranslation();
				nr++;
				for (int u = 0; u < 4; u++) {
					if (m_targets[u])
					{
						float xDst = abs(m_targets[i]->getTransform().getTranslation().x - m_targets[u]->getTransform().getTranslation().x);
						if (xDst > maxXDst)
							maxXDst = xDst;
						float yDst = abs(m_targets[i]->getTransform().getTranslation().y - m_targets[u]->getTransform().getTranslation().y);
						if (yDst > maxYDst)
							maxYDst = yDst;
					}
				}

				//extra movement / length if aiming outside of camera ? 

			}
		}
	}

	const PerspectiveCamera* cam = dynamic_cast<const PerspectiveCamera*>(getCamera());
	if (!cam) {
		Logger::Error("CAMERA IS NOT PERSPECTIVE, MAYDAY");
	}

	bool targetsStandingStill = false;

	if (nr > 0) {
		newTarget /= float(nr);
		if (newTarget == m_targetLastFrame)
			targetsStandingStill = true;
		m_targetLastFrame = newTarget;

		Vector3 moveVec = newTarget - m_target;
		m_target += moveVec * dt * m_followSpeed;
	}


	if (m_lockToMap) {
		float aspectRatio = 1.f / cam->getAspectRatio();
		float VFOVRad = cam->getFOV();
		float halfVFOVRad = VFOVRad / 2.f;
		float halfHFOVRad = atan(tan(VFOVRad / 2) / aspectRatio);

		// Clamp to map borders
		float maxMapSize = (m_mapSize.x < m_mapSize.y) ? m_mapSize.x : m_mapSize.y;
		float maxZ = (maxMapSize / 2.f) / tan(halfVFOVRad);

		// Zoom in camera further after 2 seconds if all targets are standing still
		float minZ = 30.f;
		if (targetsStandingStill && m_extraZTarget <= minZ + 0.001f) {
			m_timeZoomedIn += dt;
			if (m_timeZoomedIn >= 2.f) {
				minZ = 10.f;
			}
		} else {
			m_timeZoomedIn = 0.f;
		}

		//Calculate extra z
		if (m_useExtraZ) {
			// Smoothstepping for very smooth min and max values, max() is used to get rid of "artifact" when maxYDst is close to 0
			m_extraZTarget = Utils::smootherstep(minZ, maxZ, (maxXDst + max(maxYDst, 5.f) * cam->getAspectRatio()) * 1.3f ) * (maxZ - minZ) + minZ;
		}


		// Zoom out fast, zoom in slow
		float zoomSpeed = m_followSpeed;
		if (m_extraZTarget < m_extraZCurrent) {
			zoomSpeed = m_followSpeed * 0.4f;
		}

		// Update extraZ
		m_extraZCurrent += (dt * zoomSpeed) * (m_extraZTarget - m_extraZCurrent);


		float camZ = -getCameraPosition().z - 1;
		Vector2 min = Vector2(camZ * tan(halfHFOVRad), camZ * tan( halfVFOVRad ));
		Vector2 max = Vector2(m_mapSize.x - min.x, m_mapSize.y - min.y);

		// Clamp values
		m_target.x = (m_target.x < min.x) ? min.x : m_target.x;
		m_target.y = (m_target.y < min.y) ? min.y : m_target.y;
		m_target.x = (m_target.x > max.x) ? max.x : m_target.x;
		m_target.y = (m_target.y > max.y) ? max.y : m_target.y;
	}


	
	if (m_moving) {
		Vector3 diff = m_target - m_position;
		m_position += diff * dt * m_moveSpeed;
	}

}




