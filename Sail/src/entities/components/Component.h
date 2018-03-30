#pragma once

#include <memory>

#define SAIL_COMPONENT static int getStaticID() { \
	return reinterpret_cast<int>(&getStaticID); \
}

class Component {
public:
	typedef std::unique_ptr<Component> Ptr;
public:
	Component() {
		
	}
	virtual ~Component() {}

private:
};

