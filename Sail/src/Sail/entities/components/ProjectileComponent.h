#pragma once
#include "Component.h"

class ProjectileComponent : public Component<ProjectileComponent> {
public:
	ProjectileComponent() { ; }
	ProjectileComponent(float damage) { m_damage = damage; }
	~ProjectileComponent() { ; }

	float m_damage = 1.0f;
};
