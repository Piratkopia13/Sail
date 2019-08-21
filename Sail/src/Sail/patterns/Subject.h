#pragma once

#include "Observer.h"
#include <vector>

class Subject {

public:
	void addObserver(Observer* observer) {
		m_observers.push_back(observer);
	}
	// TODO: Remove observer method

protected:
	void notify(void* data, Event event) {
		for (Observer* observer : m_observers) {
			observer->onNotify(data, event);
		}
	}

private:
	std::vector<Observer*> m_observers;

};