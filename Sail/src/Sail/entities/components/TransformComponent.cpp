#include "pch.h"
#include "TransformComponent.h"

#include "Sail/gui/SailGuiWindow.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

void TransformComponent::renderEditorGui(SailGuiWindow* window) {
	window->enableColumns(90.f);
	window->addProperty("Translation", [&] {
		static float translation[3];
		memcpy(translation, glm::value_ptr(getTranslation()), sizeof(translation));
		if (ImGui::DragFloat3("##hideLabel", translation, 0.05f)) {
			setTranslation(glm::make_vec3(translation));
		}
	});
	window->addProperty("Rotation", [&] {
		static float rotation[3];
		memcpy(rotation, glm::value_ptr(getRotations()), sizeof(rotation));
		if (ImGui::DragFloat3("##hideLabel", rotation, 0.01f)) {
			setRotations(glm::make_vec3(rotation));
		}
	});
	window->addProperty("Scale", [&] {
		static float scale[3];
		memcpy(scale, glm::value_ptr(getScale()), sizeof(scale));
		if (ImGui::DragFloat3("##hideLabel", scale, 0.05f)) {
			setScale(glm::make_vec3(scale));
		}
	});
	window->disableColumns();
}
