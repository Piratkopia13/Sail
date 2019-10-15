#pragma once
#include "Component.h"

// NOTE: No entity should EVER add this
// It is only used to guarantee an entity is not added to a system which requires it
class NoEntityComponent final : public Component<NoEntityComponent> { };