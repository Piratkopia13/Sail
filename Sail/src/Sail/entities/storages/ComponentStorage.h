#pragma once
#include <vector>
#include "BaseComponentStorage.h"

template<typename T>
class ComponentStorage : public BaseComponentStorage {
public:
	virtual ~ComponentStorage() {}

	std::vector<T>& getComponents() {
		return components;
	}

protected:
	ComponentStorage() {}
	std::vector<T> components;
};