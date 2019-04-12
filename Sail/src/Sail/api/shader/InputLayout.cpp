#include "pch.h"
#include "InputLayout.h"
#include "Sail/Application.h"

InputLayout::InputLayout() {
	VertexSize = 0;
}

InputLayout::~InputLayout() {

}

const std::vector<InputLayout::InputType>& InputLayout::getOrderedInputs() const {
	return InputOrder;
}

UINT InputLayout::getVertexSize() const {
	return VertexSize;
}

UINT InputLayout::getInstanceSize() const {
	return InstanceSize;
}
