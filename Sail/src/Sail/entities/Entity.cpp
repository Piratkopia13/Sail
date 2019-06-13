#include "pch.h"
#include "Entity.h"

Entity::SPtr Entity::Create() {
	return std::make_shared<Entity>();
}

Entity::Entity() {

}

Entity::~Entity() {

}
