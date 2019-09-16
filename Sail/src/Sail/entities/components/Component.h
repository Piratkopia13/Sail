#pragma once
#include <memory>

typedef int ComponentTypeID;

/*
	Counter for assigning IDs to component types at compile time
	Defined in Component.cpp
*/
extern ComponentTypeID global_componentID;

/*
	Base component class
	Created components should NOT inherit from this
	This should be hidden in the future
*/
class BaseComponent {
public:
	typedef std::unique_ptr<BaseComponent> Ptr;

	virtual ~BaseComponent() {}
	
	/*
		Called by Component<T> below to assign IDs to the different types
		Should not be called anywhere else
	*/
	static ComponentTypeID createID() {
		return global_componentID++;
	}

	/*
		Retrieves the number of component types
	*/
	static int nrOfComponentTypes() {
		return global_componentID;
	}

	unsigned int entityID;

protected:
	BaseComponent() {}
};


/*
	Component class
	Created components should inherit from this and pass their own type as template
	Their type ID will be assigned automatically
*/
template<typename ComponentType>
class Component : public BaseComponent {
public:
	virtual ~Component() {}

	static const ComponentTypeID ID;
	static const unsigned int SIZE;
protected:
	Component() {}
};

/*
	Defines the constant static ID of each component type at compile time
*/
template<typename ComponentType>
const ComponentTypeID Component<ComponentType>::ID = BaseComponent::createID();

/*
	Defines the constant static size of each component type at compile time
*/
template<typename ComponentType>
const unsigned int Component<ComponentType>::SIZE = sizeof(ComponentType);