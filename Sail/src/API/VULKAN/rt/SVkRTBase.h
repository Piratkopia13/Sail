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

	// Builds/rebuilds BLASes for all submitted meshes and builds one TLAS containing all the BLASes
	// Also updates cbuffer with camera and light information
	void update(const Renderer::RenderCommandList sceneGeometry, Camera* cam, LightSetup* lights, VkCommandBuffer cmd);

	void dispatch(SVkRenderableTexture* outputTexture, Camera* cam, LightSetup* lights, VkCommandBuffer cmd);

private:
	struct InstanceList {
		SVkAPI::AccelerationStructureAllocation asAllocation;
		std::vector<glm::mat3x4> instanceTransforms; // List of instances using the same BLAS (each instance has its own transformation matrix)
	};

	struct BlasBuildInfo {
		VkAccelerationStructureCreateGeometryTypeInfoKHR geometryInfo = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
		VkAccelerationStructureGeometryKHR asGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		VkAccelerationStructureBuildOffsetInfoKHR offset = { };
		VkBuildAccelerationStructureFlagBitsKHR flags = { };
		const Mesh* mesh = nullptr; // Used as key in the bottomInstances map
		SVkAPI::AccelerationStructureAllocation* asAllocation = nullptr;
	};

private:
	InstanceList& addBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags);
	void buildBLASes(VkCommandBuffer cmd);
	void buildTLAS(uint32_t numInstances, VkBuildAccelerationStructureFlagBitsKHR flags, VkCommandBuffer cmd);
	void updateSceneCBuffer(Camera* cam, LightSetup* lights);

private:
	SVkAPI* m_context;
	VkPhysicalDeviceRayTracingPropertiesKHR m_limitProperties;

	SVkShader* m_shader;

	// One for each swap frame
	std::vector<SVkAPI::AccelerationStructureAllocation> m_tlasAllocations;
	std::vector<SVkAPI::BufferAllocation> m_instancesAllocations;
	std::vector<SVkAPI::BufferAllocation> m_tlasScratchBufferAllocations;
	std::vector<SVkAPI::BufferAllocation> m_blasScratchBufferAllocations;

	SVkAPI::BufferAllocation m_sbtAllocation; // Shader Binding Table buffer

	std::vector<BlasBuildInfo> m_blasesToBuild;

	std::vector<std::unordered_map<const Mesh*, InstanceList>> m_bottomInstances; // Each entry in the map is a unique BLAS.
																			// Each instance inside the instance list is an instance of this BLAS 
																			// with its own transformation matrix that will be put into the TLAS.
};