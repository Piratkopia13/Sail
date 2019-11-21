#pragma once

#include "Component.h"

class SpectatorComponent : public Component<SpectatorComponent> {
public:
	SpectatorComponent() {}
	~SpectatorComponent() {}

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};