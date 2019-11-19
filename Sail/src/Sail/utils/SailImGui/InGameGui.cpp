#include "pch.h"
#include "InGameGui.h"
#include "Sail/Application.h"
#include "Sail/utils/SailImGui/CustomImGuiComponents/CustomImGuiComponents.h"

#include "Sail/entities/components/SanityComponent.h"
#include "Sail/entities/components/SprintingComponent.h"

InGameGui::InGameGui(bool showWindow) {
}

InGameGui::~InGameGui() {
}

void InGameGui::renderWindow() {
	float screenWidth = Application::getInstance()->getWindow()->getWindowWidth();
	float screenHeight = Application::getInstance()->getWindow()->getWindowHeight();
	float progresbarLenght = 300;
	float progresbarHeight = 40;
	float outerPadding = 50;

	ImGui::SetNextWindowPos(ImVec2(
		screenWidth - progresbarLenght,
		screenHeight - progresbarHeight * 3
	));

	ImGui::SetNextWindowSize(ImVec2(
		progresbarLenght,
		progresbarHeight - progresbarHeight * 2.2
	));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoNav; 
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_AlwaysAutoResize; 
	flags |= ImGuiWindowFlags_NoSavedSettings;
	flags |= ImGuiWindowFlags_NoBackground;
	ImGui::Begin("GUI", NULL, flags);

	if (m_player) {

		SanityComponent* c1 = m_player->getComponent<SanityComponent>();
		SprintingComponent* c2 = m_player->getComponent<SprintingComponent>();

		if (c1) {
			float val = c1->sanity / 100.f;
			float val_inv = 1 - val;

			CustomImGui::CustomProgressBar(val, ImVec2(-1, 0), "Sanity", ImVec4(1 - val_inv * 0.3, 0.6 - val_inv * 0.6, 0, 1));
		}

		if (c2) {
			float val;
			float val_inv;
			ImVec4 color;

			if (c2->exhausted) {
				val = (c2->downTimer / MAX_SPRINT_DOWN_TIME);
				val_inv = 1 - val;
				color = ImVec4(0.5, 0.5, 0.5, 1);
			} else {
				val = 1 - (c2->sprintTimer / MAX_SPRINT_TIME);
				val_inv = 1 - val;
				color = ImVec4(1 - val_inv * 0.3, 0.6 - val_inv * 0.6, 0, 1);
			}


			CustomImGui::CustomProgressBar(val, ImVec2(-1, 0), "Stamina", color);
		}
	}

	ImGui::End();

	// Crosshair settings window


	// Crosshair
	float centerPadding = 10;
	ImVec2 crosshairSize{
		200,
		200
	};
	ImVec2 center{
		screenWidth * 0.5f,
		screenHeight * 0.5f
	};
	ImVec2 topLeft{
		center.x - crosshairSize.x * 0.5f,
		center.y - crosshairSize.y * 0.5f
	};
	ImVec2 top{ 
		topLeft.x + crosshairSize.x * 0.5f,
		topLeft.y
	};
	ImVec2 bot{
		center.x,
		center.y + crosshairSize.y * 0.5f
	};
	ImVec2 right{
		center.x + crosshairSize.x * 0.5f,
		center.y
	};
	ImVec2 left{
		center.x - crosshairSize.x * 0.5f,
		center.y
	};

	
	ImGui::SetNextWindowPos(topLeft);
	ImGui::SetNextWindowSize(crosshairSize);

	ImGuiWindowFlags crosshairFlags = ImGuiWindowFlags_NoCollapse;
	crosshairFlags |= ImGuiWindowFlags_NoResize;
	crosshairFlags |= ImGuiWindowFlags_NoMove;
	crosshairFlags |= ImGuiWindowFlags_NoNav;
	crosshairFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	crosshairFlags |= ImGuiWindowFlags_NoTitleBar;
	//crosshairFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	//crosshairFlags |= ImGuiWindowFlags_NoSavedSettings;
	crosshairFlags |= ImGuiWindowFlags_NoBackground;
	ImGui::Begin("Crosshair", NULL, crosshairFlags);

	ImGui::SliderFloat("Center padding", &centerPadding, 0, 100);

	static ImVec4 colorFloat = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
	const ImU32 color = ImColor(colorFloat);
	float thickness = 1.0f;

	ImVec2 center_padded_top{
		top.x,
		center.y - centerPadding
	};
	ImVec2 center_padded_bot{
		top.x,
		center.y + centerPadding
	};
	ImVec2 center_padded_right{
		center.x + centerPadding,
		right.y
	};
	ImVec2 center_padded_left{
		center.x - centerPadding,
		left.y
	};

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	//		|
	//   
	//
	draw_list->AddLine(
		top,
		center_padded_top,
		color,
		thickness
	);  

	//		|
	//   
	//		|
	draw_list->AddLine(
		bot,
		center_padded_bot,
		color,
		thickness
	);

	//		|
	//		    --
	//		|
	draw_list->AddLine(
		right,
		center_padded_right,
		color,
		thickness
	);  
	
	//		|
	//	--	   --
	//		|
	draw_list->AddLine(
		left,
		center_padded_left,
		color,
		thickness
	);

	ImGui::End();

}

void InGameGui::setPlayer(Entity* player) {
	m_player = player;
}

void InGameGui::setCrosshair(Entity* pCrosshairEntity) {
	m_crosshair = pCrosshairEntity;
}
