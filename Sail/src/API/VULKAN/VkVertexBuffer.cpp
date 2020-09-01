#include "pch.h"
#include "VkVertexBuffer.h"
#include "Sail/Application.h"
//#include "VkUtils.h"

VertexBuffer* VertexBuffer::Create(const Mesh::Data& modelData, bool allowUpdates) {
	return SAIL_NEW VkVertexBuffer(modelData, allowUpdates);
}

// When allowUpdates is enabled, there will exist one vertex buffer for each GPUBuffer,
// otherwise there will only be one buffer since it is not allowed to change between frames
VkVertexBuffer::VkVertexBuffer(const Mesh::Data& modelData, bool allowUpdates)
	: VertexBuffer(modelData)
{
	assert(false);
}

VkVertexBuffer::~VkVertexBuffer() { }

void VkVertexBuffer::bind(void* cmdList) {
	assert(false);
}

void VkVertexBuffer::update(Mesh::Data& data) {
	assert(false);
}

void VkVertexBuffer::setAsUpdated() {
	assert(false);
}

bool VkVertexBuffer::hasBeenUpdated() const {
	assert(false);
	return false;
}

void VkVertexBuffer::resetHasBeenUpdated() { 
	assert(false);
}