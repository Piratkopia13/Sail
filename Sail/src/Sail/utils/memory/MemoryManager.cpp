#include "MemoryManager.h"

#include <string>
#include <cstdlib>

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void* MemoryManager::getMem(unsigned int sizeBytes) {
	return calloc(1, sizeBytes);
}

void MemoryManager::addPool(PoolInstance instance) {
	PoolAllocator* temp = new PoolAllocator(
		getMem(instance.sizeBytesEachEntry * instance.numEntries),
		instance.sizeBytesEachEntry,
		instance.numEntries,
		instance.numQuadrants
	);
	std::vector<PoolAllocator*>::iterator it = m_pools.begin();
	int pos = 0;
	bool largerFound = false;

	// Check for pools with entires larger than the one being added
	for (it; it != m_pools.end(); it++) {
		if (instance.sizeBytesEachEntry < (*it)->getEntrySize()) {
			// If found a pool with larger entries, insert the new pool before the larger pool
			m_pools.insert(it, temp);
			largerFound = true;
			return;
		}
	}
	if (!largerFound) {
		// No pool found, add it to the back of the vector
		m_pools.push_back(temp);
	}
}

void MemoryManager::addStack(StackInstance instance) {
	m_stacks.push_back(new StackAllocator(getMem(instance.sizeBytes), instance.sizeBytes));
}





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

MemoryManager::MemoryManager() {}
MemoryManager::~MemoryManager() {
	cleanUp();
}

void MemoryManager::init(const std::vector<StackInstance>& stackInstances, const std::vector<PoolInstance>& poolInstances) {
	int currIndex = 0;
	for (auto SI : stackInstances) {
		addStack(SI);
		// Creating 'vector<bool>' for memory tacking purposes
		m_currMemUsage.stacks.push_back(m_stacks.at(currIndex++)->getUsedMemory());
		m_currMemUsagePercentage.stacks.push_back(0.f);
	}

	currIndex = 0;
	// Initializing each 'PoolInstance' into an actual pool in the memory manager
	for (PoolInstance PI : poolInstances) {
		if (PI.numEntries % PI.numQuadrants != 0)
			throw std::exception(("The number of entries in PoolInstance " + std::to_string(currIndex) + " was not divisible with the number of quadrants.").c_str());
		// Inserting the pool into the 'm_pools' vector
		addPool(PI);
		// Creating 'vector<bool>' for memory tacking purposes
		m_currMemUsage.pools.push_back(m_pools.at(currIndex++)->getUsedMemory());
		m_currMemUsagePercentage.stacks.push_back(0.f);
	}
}

void * MemoryManager::stackAllocate(unsigned int sizeBytes, unsigned int indexOfStack) {
	if (indexOfStack < 0 || indexOfStack > m_stacks.size() - 1)
		RM_DEBUG_MESSAGE("MemoryManager::stackAllocate: Index was out of bounds.", 1);

	void* toReturn;
	try {
		toReturn = m_stacks[indexOfStack]->allocate(sizeBytes);
		return toReturn;
	}
	catch (std::exception& e) {
		RM_DEBUG_MESSAGE("MemoryManager::stackAllocate: " + std::string(e.what()), 1);
		abort();
	}

}

void* MemoryManager::poolAllocate(unsigned int sizeBytes) {
	void* ptrToAllocation = nullptr;
	// Check for an appropriate sized pool; if it already exists
	for (auto it = m_pools.begin(); it != m_pools.end(); ++it) {
		if (sizeBytes <= (*it)->getEntrySize()) {
			ptrToAllocation = (*it)->allocate();
			break;
		}
	}

	if (ptrToAllocation == nullptr)
		throw std::exception("MemoryManager::poolAllocate : No pool allocated for size " + sizeBytes);

	return ptrToAllocation;
}

void MemoryManager::deallocateSinglePool(void* ptr, unsigned int sizeOfAlloc) {
	// We look for which pool the object exists in (ordered: smallest pool -> biggest pool)
	for (unsigned int i = 0; i < m_pools.size(); i++) {
		if (sizeOfAlloc <= m_pools.at(i)->getEntrySize()) {
			m_pools.at(i)->deallocateSingle(ptr); // We then deallocate it
			break;
		}
	}
}

void MemoryManager::deallocateAllPools() {
	for (unsigned int i = 0; i < m_pools.size(); i++)
		m_pools.at(i)->deallocateAll();
}

void MemoryManager::deallocateStack(unsigned int indexOfStack) {
	if (indexOfStack < 0 || indexOfStack > m_stacks.size() - 1)
		RM_DEBUG_MESSAGE("MemoryManager::deallocateStack: Index was out of bounds.", 1);

	m_stacks[indexOfStack]->deallocateAll();
}

void MemoryManager::deallocateStack(unsigned int indexOfStack, Marker toMarker) {
	if (indexOfStack < 0 || indexOfStack > m_stacks.size() - 1)
		RM_DEBUG_MESSAGE("MemoryManager::deallocateStack: Index was out of bounds.", 1);

	m_stacks[indexOfStack]->clearToMarker(toMarker);
}

Marker MemoryManager::getStackMarker(unsigned int indexOfStack) {
	if (indexOfStack < 0 || indexOfStack > m_stacks.size() - 1)
		RM_DEBUG_MESSAGE("MemoryManager::getStackMarker: Index was out of bounds.", 1);

	return m_stacks[indexOfStack]->getMarker();
}

void MemoryManager::updateAllocatedSpace() {
	for (unsigned int i = 0; i < m_stacks.size(); i++) {
		// Updates multiple 'vector<bool>' (used to visualize memory consumption)
		m_currMemUsage.stacks[i] = m_stacks[i]->getUsedMemory();
	}

	for (unsigned int i = 0; i < m_pools.size(); i++) {
		// Updates multiple 'vector<bool>' (used to visualize memory consumption)
		m_currMemUsage.pools[i] = m_pools.at(i)->getUsedMemory();
	}
}

MemoryUsage& MemoryManager::getAllocatedSpace() {
	return m_currMemUsage;
}

void MemoryManager::updateAllocatedSpacePercentage() {
	for (unsigned int i = 0; i < m_stacks.size(); i++) {
		m_currMemUsagePercentage.stacks[i] = m_stacks[i]->getUsedMemoryPercentage();
	}
}

MemoryUsagePercentage & MemoryManager::getAllocatedSpacePercentage() {
	return m_currMemUsagePercentage;
}

void MemoryManager::cleanUp() {
	size_t loopCount;

	loopCount = m_pools.size();
	for (unsigned int i = 0; i < loopCount; i++)
		if (m_pools.at(i) != nullptr)
			delete m_pools.at(i);
	m_pools.clear();
	m_pools.resize(0);

	for (unsigned int i = 0; i < m_stacks.size(); i++)
		if (m_stacks[i] != nullptr)
			delete m_stacks[i];
	m_stacks.clear();
	m_stacks.resize(0);

	m_currMemUsage.pools.clear();
	m_currMemUsage.stacks.clear();
}
