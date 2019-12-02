#pragma once
#include "Component.h"

// Used to identify entities that should be rendered in the killcam
class RenderInReplayComponent : public Component<RenderInReplayComponent> {
public:
	RenderInReplayComponent() {}
	virtual ~RenderInReplayComponent() {}

#ifdef DEVELOPMENT
	const unsigned int getByteSize() const override {
		return sizeof(*this);
	}
#endif
};
