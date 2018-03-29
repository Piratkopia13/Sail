#pragma once

#include "Component.h"
#include "../../utils/Utils.h"

class TestTwoComponent : public Component {
public:
	SAIL_COMPONENT

	TestTwoComponent(float a, float b) {
		Logger::Log("Two! " + std::to_string(a));
	}
	~TestTwoComponent() {}
};

