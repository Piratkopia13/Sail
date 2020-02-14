#include "pch.h"
#include "SailGuiWindow.h"

#include <commdlg.h>
#include <atlstr.h>
#include "imgui.h"
#include "Sail/utils/Utils.h"
#include "imgui_internal.h"
#include "API/Windows/Win32Window.h"
#include "Sail/Application.h"

SailGuiWindow::SailGuiWindow() 
	: setNextPropWidth(true)
	, propID(0)
	, propWidth(0)
{ }

void SailGuiWindow::addProperty(const char* label, std::function<void()> prop) {
	ImGui::PushID(propID++);
	ImGui::AlignTextToFramePadding();
	ImGui::Text(label);
	ImGui::NextColumn();
	if (setNextPropWidth)
		ImGui::SetNextItemWidth(propWidth);
	prop();
	ImGui::NextColumn();
	ImGui::PopID();
}

void SailGuiWindow::setOption(const std::string& optionName, bool value) {
	if (optionName == "setWidth") {
		setNextPropWidth = value;
	}
}

void SailGuiWindow::newSection(const std::string& title) {
	disableColumns();
	ImGui::Separator();
	ImGui::Text(title.c_str());
	enableColumns();
}

std::string SailGuiWindow::OpenFileDialog(const wchar_t* filter) {
	OPENFILENAME ofn;
	WCHAR fileName[MAX_PATH] = L"";
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = *Application::getInstance()->getWindow<Win32Window>()->getHwnd();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = L"";
	std::string fileNameStr;
	if (GetOpenFileName(&ofn))
		fileNameStr = CW2A(fileName);
	return fileNameStr;
}

void SailGuiWindow::LimitStringLength(std::string& str, int maxLength /*= 20*/) {
	if (str.length() > maxLength) {
		str = "..." + str.substr(str.length() - maxLength + 3, maxLength - 3);
	}
}

bool SailGuiWindow::DrawSplitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size) {
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	ImVec2 minAdd = (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Min.x = window->DC.CursorPos.x + minAdd.x;
	bb.Min.y = window->DC.CursorPos.y + minAdd.y;
	ImVec2 maxAdd = CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	bb.Max.x = bb.Min.x + maxAdd.x;
	bb.Max.y = bb.Min.y + maxAdd.y;
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

void SailGuiWindow::newFrame() {
	// Reset propID for imgui to function properly
	propID = 0;
}

void SailGuiWindow::enableColumns(float labelWidth) {
	ImGui::PushID(propID++);
	ImGui::Columns(2, "alignedList", false);  // 3-ways, no border
	ImGui::SetColumnWidth(0, labelWidth);
	propWidth = ImGui::GetColumnWidth(1) - 10;
	ImGui::PopID();
}

void SailGuiWindow::disableColumns() {
	ImGui::Columns(1);
}
