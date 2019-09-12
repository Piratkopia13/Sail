#pragma once
#include <vector>
#include "Component.h"

class ComponentStorage final {
public:

	/*
		Specified list of components returned by getAllComponentsOfType<ComponentType>()
	*/
	template<typename ComponentType>
	struct ComponentList {
		ComponentType* components = nullptr;
		unsigned int nrOfComponents = 0;
	};

	ComponentStorage();
	~ComponentStorage();

	/*
		Adds a component to the storage and passes any number of arguments to its constructor
	*/
	template<typename ComponentType, typename... TArgs>
	void addComponent(TArgs... args);

	/*
		Returns a specified version of the generic component list stored inside
	*/
	template<typename ComponentType>
	ComponentList<ComponentType> getAllComponentsOfType() const;

private:
	/*
		Information about a generic array of components
	*/
	struct GenericComponentList {
		BaseComponent* comps = nullptr;
		unsigned int nrOfComps = 0;
		unsigned int maxNrOfComps = 0;
	};

	/*
		Expands one of the arrays to fit more components
	*/
	template<typename ComponentType>
	void expand(GenericComponentList& list);

	std::vector<GenericComponentList> m_components;
};


template<typename ComponentType, typename... TArgs>
void ComponentStorage::addComponent(TArgs... args) {
	ComponentTypeID cType = ComponentType::ID;
	GenericComponentList* list = &m_components[cType];

	if (list->nrOfComps == list->maxNrOfComps) {
		expand<ComponentType>(*list);
	}

	// Cast the array base class pointer to a sub class pointer
	ComponentType* components = static_cast<ComponentType*>(list->comps);

	// Create the new component by using the pointer as a sub class array
	components[list->nrOfComps++] = ComponentType(args...);
}

template<typename ComponentType> ComponentStorage::ComponentList<ComponentType>
ComponentStorage::getAllComponentsOfType() const {
	ComponentList<ComponentType> list;
	list.components = static_cast<ComponentType*>(m_components[ComponentType::ID].comps);
	list.nrOfComponents = m_components[ComponentType::ID].nrOfComps;

	return list;
}

template<typename ComponentType>
void ComponentStorage::expand(ComponentStorage::GenericComponentList& list) {
	if (list.nrOfComps == list.maxNrOfComps) {
		// Increase the maximum number of components of this type
		const unsigned int incrementSize = 8;
		list.maxNrOfComps += incrementSize;

		// Create a new sub class array of the new size
		ComponentType* newComps = new ComponentType[list.maxNrOfComps];

		// Cast the base class array pointer to a sub class array pointer
		ComponentType* compsToCopy = static_cast<ComponentType*>(list.comps);

		// Copy the data into the new array
		for (unsigned int i = 0; i < list.nrOfComps; i++) {
			newComps[i] = compsToCopy[i];
		}

		// Delete the old array
		delete[] compsToCopy;

		// Assign the pointer to the new array to the base class pointer
		list.comps = newComps;
	}
}