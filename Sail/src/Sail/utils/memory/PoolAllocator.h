#ifndef POOL_ALLOCATOR
#define POOL_ALLOCATOR

#include <deque>
#include <atomic>
#include <vector>

#include "Allocator.h"

class PoolAllocator : private Allocator {
private: /// VARIABLES
	int findFreeEntry(int quadrant);
	bool checkIfAllQuadrantsSafe(const std::vector<bool>& quadrantSafe) const;

private: /// FUNCTIONS
	std::deque<bool> m_entries;

	size_t m_entrySize;
	unsigned int m_numEntries;
	unsigned int m_quadrantSize;
	unsigned int m_numQuadrants;
	unsigned int m_entriesPerQuadrant;
	unsigned int m_startQuadrant;

	// Atomic Variables
	std::deque<std::atomic_bool> m_usedQuadrants;

	// Holds each NEXT FREE ADDRESS for each quadrant
	std::vector<void*> m_quadFreeAddress;

public: /// VARIABLES

public: /// FUNCTIONS
	PoolAllocator(void* memPtr, unsigned int entrySize, unsigned int numEntries, unsigned int numQuadrants);
	virtual ~PoolAllocator();

	void* allocate();

	virtual void deallocateAll();

	void deallocateSingle(void* address);

	// Memory tracking for debugging purposes
	virtual const std::vector<bool>& getUsedMemory();

	unsigned int getEntrySize() const;

	void cleanUp();
};

#endif //POOL_ALLOCATOR
