#include "../SVkAPI.h"

#include <volk/volk.h>

class SVkShader;
class SVkRenderableTexture;
class Camera;
class LightSetup;

class SVkRTBase {
public:
	SVkRTBase();
	~SVkRTBase();

	void createBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd);
	void createTLAS(VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd);

	void dispatch(SVkRenderableTexture* outputTexture, Camera* cam, LightSetup* lights, VkCommandBuffer cmd);

private:
	SVkAPI* m_context;
	VkPhysicalDeviceRayTracingPropertiesKHR m_limitProperties;

	SVkShader* m_shader;

	SVkAPI::AccelerationStructureAllocation m_blasAllocation;
	SVkAPI::AccelerationStructureAllocation m_tlasAllocation;
	SVkAPI::BufferAllocation m_instancesAllocation;

	SVkAPI::BufferAllocation m_sbtAllocation; // Shader Binding Table buffer
};