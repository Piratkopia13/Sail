#pragma once

#include "Sail.h"

class Kalaha {
public:
	Kalaha();
	~Kalaha();

	void reset();
	bool play(int houseNumber);
	int getHouseCount(int player, int houseNumber) const;
	int getStoreCount(int player) const;
	int getCurrentTurn() const;
	bool isGameOver() const;

private:

	int m_currentTurn;
	bool m_gameOver;

	int m_houses[2][6];
	int m_stores[2];

};