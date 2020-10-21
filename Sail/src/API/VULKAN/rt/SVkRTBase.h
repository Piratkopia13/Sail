#include "../SVkAPI.h"

#include <volk/volk.h>

class SVkRTBase {
public:
	SVkRTBase();

	void createBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd);
	void createTLAS(VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd);

private:
	SVkAPI* m_context;
	VkPhysicalDeviceRayTracingPropertiesKHR m_limitProperties;


	SVkAPI::AccelerationStructureAllocation m_blasAllocation;
	SVkAPI::AccelerationStructureAllocation m_tlasAllocation;
	SVkAPI::BufferAllocation m_instancesAllocation;
};