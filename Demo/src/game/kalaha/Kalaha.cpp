#include "Kalaha.h"

#define PLAYER1 0
#define PLAYER2 1

Kalaha::Kalaha() {
	
	// Initialize game
	reset();

}

Kalaha::~Kalaha() {

}

void Kalaha::reset() {

	m_currentTurn = 0;

	for(int i = 0; i < 6; i++) {
		m_houses[PLAYER1][i] = 4;
		m_houses[PLAYER2][i] = 4;
	}
	m_stores[PLAYER1] = 0;
	m_stores[PLAYER2] = 0;

	m_gameOver = false;

}

bool Kalaha::play(int houseNumber) {

	// Discard invalid plays
	if (houseNumber < 0 || houseNumber > 5)
		return false;

	// Number of seeds in the selected house
	int& seedsInHouse = m_houses[m_currentTurn][houseNumber];

	// Discard play if there are no seeds
	if (seedsInHouse == 0)
		return false;

	bool extraTurn = false;
	// Make copies to modify without changing the originals
	int house = houseNumber;
	int turn = m_currentTurn;
	// Iterate seeds and place one in house at every step
	for (; seedsInHouse > 0; seedsInHouse--) {

		house++;
		if (house == 6) {

			house = -1; // Will become 0 next iteration
			if (turn == m_currentTurn) {
				// Add seed to your store
				m_stores[m_currentTurn]++;
				// Give the player an extra turn if this was the last seed
				if (seedsInHouse == 1)
					extraTurn = true;
			}
			else {
				// Other players store - skip it by adding an iteration to the loop
				seedsInHouse++;
			}
			// Switch turn (to put seeds on opposite side)
			turn = (turn == 0) ? 1 : 0;

		} else {

			if (turn == m_currentTurn && m_houses[turn][house] == 0 && seedsInHouse == 1) {
				// Capture opponents seeds
				int otherPlayer = (turn == 0) ? 1 : 0;;
				m_stores[turn] += m_houses[otherPlayer][5 - house] + 1;
				m_houses[otherPlayer][5 - house] = 0;
			} else {
				// Add seed to house
				m_houses[turn][house]++;
			}

		}

	}

	// Game over condition - no seeds in any of a players houses
	m_gameOver = true;
	for (int i = 0; i < 6; i++) {
		if (m_houses[m_currentTurn][i] > 0)
			m_gameOver = false;
	}


	if (!extraTurn) {
		// Turn over
		m_currentTurn = (m_currentTurn == 0) ? 1 : 0;
	}


	return true;

}

int Kalaha::getHouseCount(int player, int houseNumber) const {

	if (houseNumber < 0 || houseNumber > 5 || player < 0 || player > 1)
		return -1;

	return m_houses[player][houseNumber];
}

int Kalaha::getStoreCount(int player) const {

	if (player < 0 || player > 1)
		return -1;

	return m_stores[player];

}

int Kalaha::getCurrentTurn() const {
	return m_currentTurn;
}

bool Kalaha::isGameOver() const {
	return m_gameOver;
}

