#pragma once

#include "Component.h"
#include "Sail/api/Mesh.h"

class RelationshipComponent : public Component {
public:
	typedef std::shared_ptr<RelationshipComponent> SPtr;
public:
	SAIL_COMPONENT
	RelationshipComponent() = default;
	~RelationshipComponent() = default;

	std::size_t numChildren{};
	// TODO: change all these to Weak ptr (or uint with entt)
	Entity::SPtr first;
	Entity::SPtr prev;
	Entity::SPtr next;
	Entity::SPtr parent;

private:
};