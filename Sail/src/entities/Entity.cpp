#include "Entity.h"

Entity::Ptr Entity::Create() {
	return std::make_unique<Entity>();
}

Entity::Entity() {

}

Entity::~Entity() {

}
