#pragma once

#include "Component.h"
#include "../../utils/Utils.h"

class TestComponent : public Component {
public:
	SAIL_COMPONENT

	TestComponent(float a) {
		Logger::Log(std::to_string(a));
	}
	~TestComponent() {}
};

