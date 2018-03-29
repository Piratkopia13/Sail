#pragma once

#include <typeinfo>
#include <type_traits>

#define SAIL_COMPONENT static int getStaticID() { \
	return reinterpret_cast<int>(&getStaticID); \
}

class Component {
public:
	Component() {
		
	}
	virtual ~Component() {}

private:
};

