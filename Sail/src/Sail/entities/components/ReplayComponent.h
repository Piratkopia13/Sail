#pragma once
#include "Component.h"


// Used to identify entities that are included in the killcam
class ReplayComponent : public Component<ReplayComponent> {
public:
	ReplayComponent() {}
	virtual ~ReplayComponent() {}
};