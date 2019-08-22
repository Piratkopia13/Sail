#include "Defines.h"
#include "PoolAllocator.h"

#include "pch.h"

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

int PoolAllocator::findFreeEntry(int quadrant) {
	int returnValue;

	// Get the next free address
	void* allocationAddress = m_quadFreeAddress.at(quadrant);

	// Calculate corresponding entry number to the free address we're taking
	unsigned int entryNum = static_cast<char*>(allocationAddress) - static_cast<char*>(m_memPtr);
	entryNum = entryNum / m_entrySize;
	returnValue = entryNum;
	// Set that entry to 'allocated'
	m_entries.at(entryNum) = true;

	// 'startEntry' refers to the first entry IN THE QUADRANT
	unsigned int startEntry = quadrant * m_entriesPerQuadrant;
	unsigned int entryNumOffset = entryNum - startEntry;

	// Initialize to nullptr (no new entry found = full quadrant = return nullptr)
	m_quadFreeAddress.at(quadrant) = nullptr;
	// We are looking for the next free entry
	for (unsigned int i = 0; i < m_entriesPerQuadrant; i++) {
		entryNumOffset++;
		entryNumOffset %= m_entriesPerQuadrant;

		if (!m_entries.at(startEntry + entryNumOffset)) {
			m_quadFreeAddress.at(quadrant) = static_cast<char*>(m_memPtr) + (entryNumOffset + startEntry) * m_entrySize;
			break;
		}
	}

	// 'returnValue' = 'entryNum' OR nullptr
	return returnValue;
}

bool PoolAllocator::checkIfAllQuadrantsSafe(const std::vector<bool>& quadrantSafe) const {
	for (unsigned int i = 0; i < quadrantSafe.size(); i++)
		if (quadrantSafe.at(i) == false)
			return false;

	return true;
}





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

PoolAllocator::PoolAllocator(void* memPtr, unsigned int entrySize, unsigned int numEntries, unsigned int numQuadrants)
	: Allocator(memPtr, entrySize * numEntries) {
	// Size of OBJECT + PADDING must be a multiple of 4 (32-bit) or 8 (64-bit)
	if (m_entrySize % ARCH_BYTESIZE != 0)
		throw std::exception("PoolAllocator::PoolAllocator(): Entry size was not a multiple of " + ARCH_BYTESIZE);

	m_entrySize = entrySize;
	m_numEntries = numEntries;
	m_numQuadrants = numQuadrants;
	m_entriesPerQuadrant = numEntries / numQuadrants;
	m_startQuadrant = 0;

	// Initially set all entries to FALSE (AKA: Not currently allocated)
	m_entries.resize(numEntries);
	for (unsigned int i = 0; i < numEntries; i++)
		m_entries.at(i) = false;

	// Initially set a mutex lock for each quadrant to FALSE (AKA: Not currentlly in use by a thread)
	m_usedQuadrants.resize(numQuadrants);
	for (unsigned int i = 0; i < m_usedQuadrants.size(); i++)
		m_usedQuadrants[i] = ATOMIC_VAR_INIT(false);

	// Calculate the size of a quadrant
	m_quadrantSize = static_cast<int>((static_cast<float>(m_sizeBytes) / static_cast<float>(m_numQuadrants)));

	// Set each quadrant's 'FreeAddress' to its own first entry
	m_quadFreeAddress.resize(numQuadrants);
	for (unsigned int i = 0; i < m_numQuadrants; i++)
		m_quadFreeAddress.at(i) = (static_cast<char*>(m_memPtr) + (m_quadrantSize * i));
}

PoolAllocator::~PoolAllocator() {
	cleanUp();
}

void* PoolAllocator::allocate() {
	// We're looking for a quadrant that's not being searched (= false)
	bool expected = false;
	// Start at the next quadrant in line
	// Optimizes the use of quadrants
	m_startQuadrant++;
	m_startQuadrant %= m_numQuadrants;
	int currentQuadrant = m_startQuadrant;
	//int currentQuadrant = 0;
	// A returnValue of '-1' means the quadrant if completely full
	int entryReturnNum = -1;

	while (entryReturnNum == -1) {
		// If the quadrant's 'm_usedQuadrants' == true, that means another thread is
		// searching in that quadrant already. So we look through the next
		// quadrant in the hopes that it's not being searched through.
		// also checks whether or not the quadrant has a free address
		bool full = false;
		while (!m_usedQuadrants.at(currentQuadrant).compare_exchange_strong(expected, true)
			|| (full = (m_quadFreeAddress.at(currentQuadrant) == nullptr))) {

			// Optimization - Removed due to high amounts of overhead
			// Throws if too much time have been taken during allocation
			//if (std::chrono::system_clock::now() > sleepTill)
			//	throw std::exception("All quadrants were full or in use for too long, initialize the pool with more memory.");

			// Check to see if the quadrant was skipped due to it being full
			if (full)
				m_usedQuadrants.at(currentQuadrant).store(false);
			expected = false;

			currentQuadrant++;
			currentQuadrant %= m_numQuadrants;
		}

		entryReturnNum = findFreeEntry(currentQuadrant);
		// The quadrant is no longer in use by a thread
		m_usedQuadrants[currentQuadrant].store(false);
	}
	// Calculating which address we have allocated, casting it to a void*
	char* returnAddress = static_cast<char*>(m_memPtr);
	returnAddress += (entryReturnNum * m_entrySize);
	return static_cast<void*>(returnAddress);
}

void PoolAllocator::deallocateAll() {
	// Keep track of which quadrant is safe (from this function's perspective)
	std::vector<bool> quadrantSafe;
	quadrantSafe.resize(m_numQuadrants);
	// Initially, this function can't be certain that any quadrants are safe
	for (unsigned int i = 0; i < quadrantSafe.size(); i++)
		quadrantSafe.at(i) = false;
	// Until all members of the vector 'quadrantSafe', we keep running this loop
	while (!checkIfAllQuadrantsSafe(quadrantSafe)) {	// If a quadrant is not in use, we set it to used AND we now know it's safe
		for (unsigned int i = 0; i < m_usedQuadrants.size(); i++)
			if (m_usedQuadrants.at(i) == false) {	// Set to 'used'
				m_usedQuadrants.at(i).store(true);
				// Set to 'safe'
				quadrantSafe.at(i) = true;
			}
	}
	// 'Deallocating' all entries
	for (auto& i : m_entries)
		i = false;

	// Set all new freeEntries (for each quadrant)
	for (unsigned int i = 0; i < m_numQuadrants; i++)
		m_quadFreeAddress.at(i) = (static_cast<char*>(m_memPtr) + (m_quadrantSize * i));
}

void PoolAllocator::deallocateSingle(void* address) {	/// STEP 1 - ACQUIRE CORRESPONDING ENTRY TO ADDRESS-PARAMETER
	///__________________________________________________________
	char* startPoint;
	char* endPoint;

	startPoint = static_cast<char*>(m_memPtr);
	endPoint = static_cast<char*>(address);
	// Calculates which entry we are deallocating 
	int entryIndex = static_cast<int>(static_cast<float>((endPoint - startPoint) / static_cast<float>(m_entrySize)));
	// Setting the entry to false = deallocation
	m_entries.at(entryIndex) = false;

	/// STEP 2 - SET NEW 'NEXT-FREE' ENTRY
	///___________________________________
	int quadrantEntryCount = static_cast<int>(static_cast<float>(m_numEntries / float(m_numQuadrants)));
	// Check which quadrant we are in
	int currentQuadrant = static_cast<int>(static_cast<float>(entryIndex / quadrantEntryCount));

	// Wait for the quadrant to be free of other threads
	bool expected = false;
	while (!m_usedQuadrants[currentQuadrant].compare_exchange_strong(expected, true))
		;//DO NOTHING

	// Set the specific quadrant's (that we just deallocated from) newest free
	// entry to the address we just deallocated.
	m_quadFreeAddress.at(currentQuadrant) = address;
	m_usedQuadrants.at(currentQuadrant).store(false);
}

const std::vector<bool>& PoolAllocator::getUsedMemory() {
	std::vector<bool> usedMemory;
	usedMemory.resize(m_entries.size());

	for (unsigned int i = 0; i < m_entries.size(); i++)
		usedMemory.at(i) = m_entries[i];
	// Returns an array with 'TRUE' or 'FALSE' for EACH ENTRY SLOT,
	// giving an overview of how full a pool is.
	return usedMemory;
}

unsigned int PoolAllocator::getEntrySize() const {
	return static_cast<unsigned int>(m_entrySize);
}

void PoolAllocator::cleanUp() {
	if (m_memPtr != nullptr) {
		free(m_memPtr);
		m_memPtr = nullptr;
	}
}