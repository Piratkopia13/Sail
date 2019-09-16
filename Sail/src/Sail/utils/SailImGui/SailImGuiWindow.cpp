#include "pch.h"
#include "SailImGuiWindow.h"

SailImGuiWindow::SailImGuiWindow() :
m_windowState(false)
{

}

SailImGuiWindow::SailImGuiWindow(const bool state) : 
m_windowState(state)
{

}

SailImGuiWindow::~SailImGuiWindow() {
}

const bool SailImGuiWindow::windowOpen()
{
	return m_windowState;
}

void SailImGuiWindow::toggle() {
	m_windowState = !m_windowState;
}

void SailImGuiWindow::windowState(const bool state) {
	m_windowState = state;
}
