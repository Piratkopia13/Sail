#pragma once

#include "SailImGui/SailImGuiWindow.h"

class Profiler : public SailImGuiWindow {

public:
	Profiler();
	explicit Profiler(bool showWindow);
	~Profiler();

	unsigned int workSetUsage() const;
	unsigned int virtMemUsage() const;
	double processUsage();
	unsigned int vramUsage() const;
	unsigned int vramBudget() const;

	virtual void renderWindow() override;

private:
	ULARGE_INTEGER m_lastCPU, m_lastSysCPU, m_lastUserCPU;
	int m_numProcessors;
	HANDLE m_self;

	double m_lastCPUpercent;


	void init();
};