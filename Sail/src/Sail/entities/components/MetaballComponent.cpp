#include "pch.h"
#include "MetaballComponent.h"

MetaballComponent::MetaballComponent(float radius) :
	m_radius(radius),
	m_material(Material(nullptr))
{

}
MetaballComponent::~MetaballComponent() {

};

float MetaballComponent::getRadius() const {
	return m_radius;
}

void MetaballComponent::setRadius(float radius) {
	m_radius = radius;
}
