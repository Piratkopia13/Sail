#pragma once
#include "Component.h"

// Entities with this component will not render with interpolated positions but will
// use their last position instead so there's no input latency for them
// Example entity: The candle that the player is holding
class RealTimeComponent : public Component<RealTimeComponent> {
public:
	RealTimeComponent() {}
	virtual ~RealTimeComponent() {}
};