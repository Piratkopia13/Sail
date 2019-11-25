#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"
#include "Sail/events/EventReceiver.h"
#include "Sail/entities/Entity.h"

class Camera;
class CameraController;

class KillCamCameraSystem final : public BaseComponentSystem, public EventReceiver {
public:
	KillCamCameraSystem();
	~KillCamCameraSystem();

	void stop() override;
	void initialize(Camera* cam);
	void update(float dt, float alpha);

	bool onEvent(const Event& event) override;


#ifdef DEVELOPMENT
	unsigned int getByteSize() const override {
		return BaseComponentSystem::getByteSize() + sizeof(*this);
	}
#endif

private:
	void updateCameraPosition(float alpha);

private:
	CameraController* m_cam = nullptr;
};