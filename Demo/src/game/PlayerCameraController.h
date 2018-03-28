#pragma once
#include "Sail.h"
#include "objects/common/Object.h"

class PlayerCameraController : public CameraController {
public:
	PlayerCameraController(Camera* cam, const DirectX::SimpleMath::Vector2* mapSize = nullptr);

	void update(float dt);
	void setTargets(Object* focusObject1, Object* focusObject2 = nullptr, Object* focusObject3 = nullptr, Object* focusObject4 = nullptr);
	void setPosition(DirectX::SimpleMath::Vector3 pos);
	void setTarget(DirectX::SimpleMath::Vector3 pos);
	void setOffset(DirectX::SimpleMath::Vector3 offset);
	void setMoveSpeed(float speed);
	void setFollowSpeed(float speed);
	void setMoving(bool moving);
	void setUseExtraZ(bool use);


	DirectX::SimpleMath::Vector3 getPosition();
	DirectX::SimpleMath::Vector3 getTarget();

private:
	void updatePosition(float dt);

	Object* m_targets[4];
	
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_target;

	DirectX::SimpleMath::Vector3 m_back;
	DirectX::SimpleMath::Vector3 m_up;

	bool m_lockToMap;
	DirectX::SimpleMath::Vector2 m_mapSize;

	bool m_useExtraZ;
	bool m_moving;
	float m_followSpeed;
	float m_moveSpeed;
	float m_cameraZOffset;
	float m_extraZCurrent;
	float m_extraZTarget;
	float m_cameraYOffset;

	float m_timeZoomedIn;
	DirectX::SimpleMath::Vector3 m_targetLastFrame;

};