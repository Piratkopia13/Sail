#include "pch.h"
#include "ComponentStorage.h"

ComponentStorage::ComponentStorage() {
	m_components.reserve(BaseComponent::nrOfComponentTypes());
	for (size_t i = 0; i < BaseComponent::nrOfComponentTypes(); i++) {
		m_components.push_back(GenericComponentList());
	}
}

ComponentStorage::~ComponentStorage() {
	for (size_t i = 0; i < m_components.size(); i++) {
		delete[] m_components[i].comps;
	}
}