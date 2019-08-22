#ifndef MEMORY_MANAGER
#define MEMORY_MANAGER

#include <vector>

#include "PoolAllocator.h"
#include "StackAllocator.h"
#include <thread>

// DEFINITIONS
// --------------------------------------
struct MemoryUsage { // All vectors of bools are used to visualize memory usage
	std::vector<std::vector<bool>> stacks;
	std::vector<std::vector<bool>> pools;
};

struct MemoryUsagePercentage {
	std::vector<float> stacks;
	std::vector<float> pools;
};

struct PoolInstance {
	unsigned int sizeBytesEachEntry;
	unsigned int numEntries;
	unsigned int numQuadrants;
};

struct StackInstance {
	unsigned int sizeBytes;
};

// ---------------------------------------

class MemoryManager {
private: /// VARIABLES
	std::vector<PoolAllocator*> m_pools;
	std::vector<StackAllocator*> m_stacks;

	MemoryUsage m_currMemUsage;
	MemoryUsagePercentage m_currMemUsagePercentage;

private: /// FUNCTIONS
	void* getMem(unsigned int sizeBytes);

	// Singleton class shouldn't be able to be copied
	MemoryManager(MemoryManager const&) = delete;
	void operator=(MemoryManager const&) = delete;

	void addPool(PoolInstance instance);
	void addStack(StackInstance instance);

public: /// FUNCTIONS

	static MemoryManager& getInstance() {
		static MemoryManager instance;

		return instance;
	}

	MemoryManager();
	~MemoryManager();

	void init(const std::vector<StackInstance>& stackInstances, const std::vector<PoolInstance>& poolInstances);

	void* stackAllocate(unsigned int sizeBytes, unsigned int indexOfStack);
	void* poolAllocate(unsigned int sizeBytes);

	void deallocateSinglePool(void* ptr, unsigned int sizeOfAlloc);
	void deallocateAllPools();
	void deallocateStack(unsigned int indexOfStack);
	void deallocateStack(unsigned int indexOfStack, Marker toMarker);

	Marker getStackMarker(unsigned int indexOfStack);

	void updateAllocatedSpace();
	MemoryUsage& getAllocatedSpace();

	void updateAllocatedSpacePercentage();
	MemoryUsagePercentage& getAllocatedSpacePercentage();

	void cleanUp();
};

#endif //MEMORY_MANAGER
