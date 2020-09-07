#include "pch.h"
#include "VkIndexBuffer.h"
//#include "VkUtils.h"
#include "Sail/Application.h"

IndexBuffer* IndexBuffer::Create(Mesh::Data& modelData) {
	return SAIL_NEW VkIndexBuffer(modelData);
}

VkIndexBuffer::VkIndexBuffer(Mesh::Data& modelData) 
	: IndexBuffer(modelData)
{
	//assert(false);
	Logger::Warning("Tried to create an index buffer");
}

VkIndexBuffer::~VkIndexBuffer() {
}

void VkIndexBuffer::bind(void* cmdList) {
	assert(false);
}