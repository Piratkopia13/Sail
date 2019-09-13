#pragma once
#include <vector>
#include <unordered_map>
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
	void addComponent(int entityID, TArgs... args);

	/*
		Removes a component from the storage
	*/
	template<typename ComponentType>
	void removeComponent(int entityID);

	/*
		Returns a component attached to an entity
	*/
	template<typename ComponentType>
	ComponentType* getComponent(int entityID);

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
		std::unordered_map<int, int> ID_to_index;
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
inline void ComponentStorage::addComponent(int entityID, TArgs... args) {
	ComponentTypeID cType = ComponentType::ID;
	GenericComponentList* list = &m_components[cType];
	
	// Do not add this component if a link between this entity and a component of this type already exists
	std::unordered_map<int, int>::iterator it = list->ID_to_index.find(entityID);
	if (it != list->ID_to_index.end()) {
		return;
	}

	if (list->nrOfComps == list->maxNrOfComps) {
		expand<ComponentType>(*list);
	}

	// Cast the base class pointer to a sub class pointer
	ComponentType* components = static_cast<ComponentType*>(list->comps);

	// Create the new component by using the pointer as a sub class array
	components[list->nrOfComps] = ComponentType(args...);

	// Link this entity ID to this component index
	list->ID_to_index[entityID] = list->nrOfComps;

	list->nrOfComps++;
}

template<typename ComponentType>
inline void ComponentStorage::removeComponent(int entityID) {
	ComponentTypeID cType = ComponentType::ID;
	GenericComponentList* list = &m_components[cType];

	// Do not remove this component unless a link between this entity and a component of this type exists
	std::unordered_map<int, int>::iterator it = list->ID_to_index.find(entityID);
	if (it == list->ID_to_index.end()) {
		return;
	}

	// Cast the base class pointer to a sub class pointer
	ComponentType* components = static_cast<ComponentType*>(list->comps);
	
	// Copy the last component's data into the one to be removed
	components[it->second] = components[list->nrOfComps - 1];

	// Redirect the link of the moved component
	// Since the last component in the array now has the index (it->second),
	// the link between its entity and itself has to be redirected
	list->ID_to_index.at(components[it->second].entityID) = it->second;

	// Remove the link between entity and component
	list->ID_to_index.erase(entityID);

	list->nrOfComps--;
}

template<typename ComponentType>
inline ComponentType* ComponentStorage::getComponent(int entityID) {
	GenericComponentList* list = &m_components[ComponentType::ID];
	
	// Check if a link between this entity ID and this component type already exists
	std::unordered_map<int, int>::iterator it = list.ID_to_index.find(entityID);
	if (it != list->ID_to_index.end()) {

		// Convert the pointer and retrieve the linked index
		ComponentType* comps = static_cast<ComponentType*>(list->comps);
		return comps[it->second];
	}
	return nullptr;
}

template<typename ComponentType> ComponentStorage::ComponentList<ComponentType>
inline ComponentStorage::getAllComponentsOfType() const {
	ComponentList<ComponentType> list;
	list.components = static_cast<ComponentType*>(m_components[ComponentType::ID].comps);
	list.nrOfComponents = m_components[ComponentType::ID].nrOfComps;

	return list;
}

template<typename ComponentType>
inline void ComponentStorage::expand(ComponentStorage::GenericComponentList& list) {
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