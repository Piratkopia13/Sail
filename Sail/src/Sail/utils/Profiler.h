#pragma once

#include <windows.h>
#include "SailImGui/SailImGuiWindow.h"

class Profiler : public SailImGuiWindow {

public:
	Profiler();
	Profiler(const bool windowState);
	~Profiler();

	unsigned int workSetUsage() const;
	unsigned int virtMemUsage() const;
	double processUsage();
	unsigned int vramUsage() const;
	unsigned int vramBudget() const;

private:
	ULARGE_INTEGER m_lastCPU, m_lastSysCPU, m_lastUserCPU;
	int m_numProcessors;
	HANDLE m_self;

	double m_lastCPUpercent;


	void init();
};