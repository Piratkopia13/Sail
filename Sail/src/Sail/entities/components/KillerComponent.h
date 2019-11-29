#pragma once
#include "Component.h"

// Used to identify the player who killed us
class KillerComponent : public Component<KillerComponent> {
public:
	KillerComponent() {}
	virtual ~KillerComponent() {}

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};
