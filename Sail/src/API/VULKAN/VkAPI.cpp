#include "pch.h"
#include "VkAPI.h"

GraphicsAPI* GraphicsAPI::Create() {
	return SAIL_NEW VkAPI();
}

VkAPI::VkAPI() {
	Logger::Log("Initializing Vulkan..");
}

bool VkAPI::init(Window* window) {
	throw std::logic_error("The method or operation is not implemented.");
}

void VkAPI::clear(const glm::vec4& color) {
	throw std::logic_error("The method or operation is not implemented.");
}

void VkAPI::setDepthMask(DepthMask setting) {
	throw std::logic_error("The method or operation is not implemented.");
}

void VkAPI::setFaceCulling(Culling setting) {
	throw std::logic_error("The method or operation is not implemented.");
}

void VkAPI::setBlending(Blending setting) {
	throw std::logic_error("The method or operation is not implemented.");
}

void VkAPI::present(bool vsync /*= false*/) {
	throw std::logic_error("The method or operation is not implemented.");
}

unsigned int VkAPI::getMemoryUsage() const {
	throw std::logic_error("The method or operation is not implemented.");
}

unsigned int VkAPI::getMemoryBudget() const {
	throw std::logic_error("The method or operation is not implemented.");
}

bool VkAPI::onResize(WindowResizeEvent& event) {
	throw std::logic_error("The method or operation is not implemented.");
}
