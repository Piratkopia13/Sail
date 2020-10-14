#pragma once

class RelationshipComponent {
public:
	RelationshipComponent() = default;
	RelationshipComponent(const RelationshipComponent&) = default;

	std::size_t numChildren{};
	// TODO: change all these to Weak ptr (or uint with entt)
	Entity::ID first;
	Entity::ID prev;
	Entity::ID next;
	Entity::ID parent;

};