#include "pch.h"
#include "SailImGuiWindow.h"

SailImGuiWindow::SailImGuiWindow()
	: m_isWindowOpen(false)
{ }

SailImGuiWindow::SailImGuiWindow(bool showWindow)
	: m_isWindowOpen(showWindow)
{ }

SailImGuiWindow::~SailImGuiWindow() {
}

bool SailImGuiWindow::isWindowOpen() {
	return m_isWindowOpen;
}

void SailImGuiWindow::toggleWindow() {
	m_isWindowOpen = !m_isWindowOpen;
}

void SailImGuiWindow::showWindow(bool show) {
	m_isWindowOpen = show;
}
