#ifndef ALLOCATOR
#define ALLOCATOR

#include <vector>

class Allocator {

protected:
	void* m_memPtr;
	size_t m_sizeBytes;


public:
	Allocator(void* memPtr, unsigned int sizeBytes);
	virtual ~Allocator();

	virtual void deallocateAll() = 0;

	// Memory tracking for debugging purposes
	virtual const std::vector<bool>& getUsedMemory() = 0;

};

#endif //ALLOCATOR
