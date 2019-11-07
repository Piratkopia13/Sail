#include "pch.h"
#include "Component.h"
#include "Sail/entities/Entity.h"

ComponentTypeID global_componentID = 0;
#ifdef DEVELOPMENT
void BaseComponent::imguiRender(Entity** selected) {
	ImGui::Text("Nothing here yet...");
	ImGui::Text("Create");
	ImGui::Separator();
	ImGui::Text("   #ifdef DEVELOPMENT");
	ImGui::Text("   void imguiRender() { ");
	ImGui::Text("      //imgui content");
	ImGui::Text("   }");
	ImGui::Text("   #endif");
	ImGui::Separator();
	ImGui::Text("in component to render stuff here");
}
#endif