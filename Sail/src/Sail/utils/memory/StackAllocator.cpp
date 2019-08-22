#include "Defines.h"
#include "StackAllocator.h"

#include "pch.h"

#include <cstdlib>

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

unsigned int StackAllocator::padMemory(unsigned int sizeBytes) {
	unsigned int paddedMemory = sizeBytes % ARCH_BYTESIZE;
	// If padding needed, determine and add padding
	if (paddedMemory != 0)
		paddedMemory = sizeBytes + (ARCH_BYTESIZE - paddedMemory);
	// Else we don't need padding
	else
		paddedMemory = sizeBytes;

	return paddedMemory;
}

void StackAllocator::cleanUp() {
	if (m_memPtr != nullptr) {
		free(m_memPtr);
		m_memPtr = nullptr;
	}
}





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/


StackAllocator::StackAllocator(void * memPtr, unsigned int sizeBytes) : Allocator(memPtr, padMemory(sizeBytes)) {
	m_marker.store(0);
}

StackAllocator::~StackAllocator() {
	cleanUp();
}

void* StackAllocator::allocate(unsigned int sizeBytes) {
	char* ptrToStackTop = nullptr;
	unsigned int paddedAllocation = padMemory(sizeBytes);

	if (m_marker + paddedAllocation <= m_sizeBytes) {
		// Get current marker location and move marker to top (where we're currently allocating)
		Marker currMarker = m_marker.fetch_add(paddedAllocation);

		ptrToStackTop = static_cast<char*>(m_memPtr) + currMarker;
	}
	else // Else if we're allocating past the max size of the stack
		throw std::overflow_error("StackAllocator::allocate : Tried to allocate more memory than was available.");

	return ptrToStackTop;
}

void StackAllocator::deallocateAll() {
	m_marker.store(0);
}

Marker StackAllocator::getMarker() {
	return m_marker;
}

void StackAllocator::clearToMarker(Marker marker) {
	m_marker.store(marker);
}

const std::vector<bool>& StackAllocator::getUsedMemory() {
	// A vector of bools that represents used memory
	std::vector<bool> usedMemory;
	usedMemory.resize(100);
	// Calculate percentage of total memory used
	unsigned int percentUsed = int(float(m_marker) / float(m_sizeBytes) * 100.f);
	// As long as 'i' is < 'percentUsed', we continue setting 'true' (to-be-drawn)
	for (unsigned int i = 0; i < 100; i++)
		usedMemory.at(i) = ((i < percentUsed) ? true : false);

	return usedMemory;
}

float StackAllocator::getUsedMemoryPercentage() {
	// Percentage of used memory
	return float(m_marker) / float(m_sizeBytes);
}
