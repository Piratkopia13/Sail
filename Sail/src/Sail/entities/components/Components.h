#pragma once

#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "RelationshipComponent.h"
#include "MeshComponent.h"
#include "TransformComponent.h"
#include "MaterialComponent.h"

struct NameComponent {
	NameComponent(const std::string& name = "Unnamed") : name(name) { };
	NameComponent(const NameComponent&) = default;

	std::string name;
};

struct IsBeingRenderedComponent {
	IsBeingRenderedComponent(bool isBeingRendered = false) : isBeingRendered(isBeingRendered) { };
	IsBeingRenderedComponent(const IsBeingRenderedComponent&) = default;

	bool isBeingRendered;
};

struct IsSelectedComponent {
	IsSelectedComponent(bool isSelected = false) : isSelected(isSelected) { };
	IsSelectedComponent(const IsSelectedComponent&) = default;

	bool isSelected;
};

struct SkyboxComponent {
	SkyboxComponent() { };
	SkyboxComponent(const SkyboxComponent&) = default;

	std::string environmentName = "not used?";
};