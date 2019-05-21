#pragma once

class ImGuiHandler {
public:
	static ImGuiHandler* Create();

	ImGuiHandler() {}
	virtual ~ImGuiHandler() {}

	virtual void init() = 0;

	virtual void begin() = 0;
	virtual void end() = 0;
};