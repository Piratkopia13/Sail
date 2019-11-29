#pragma once
#include "Component.h"

// Used to identify entities that should be rendered during normal gameplay
class RenderInActiveGameComponent : public Component<RenderInActiveGameComponent> {
public:
	RenderInActiveGameComponent() {}
	virtual ~RenderInActiveGameComponent() {}

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};
