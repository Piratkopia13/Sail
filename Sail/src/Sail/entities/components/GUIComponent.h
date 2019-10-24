#pragma once
#include "Component.h"

class GUIComponent : public Component<GUIComponent> {
public:
	GUIComponent() { }
	~GUIComponent() { }
};
