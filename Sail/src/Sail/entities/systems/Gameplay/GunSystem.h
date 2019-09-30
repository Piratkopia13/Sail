#pragma once
#include "..//BaseComponentSystem.h"
#include "Sail/graphics/Scene.h"

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void setScene(Scene* scene);

	void update(float dt) override;

private:
	Scene* m_scene;

};
