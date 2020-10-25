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

	// Info required to create the compacted blas
	struct BlasCompactInfo {
		std::vector<BlasBuildInfo> buildInfo; // Only flags and asAllocation are used from this, TODO: consider storing only those
		uint32_t totalUncompactedSize = 0;
		VkQueryPool queryPool = { };
	};
	// Info required to replace the un-compacted blas with the compacted one
	struct CompactedBlasInfo {
		SVkAPI::AccelerationStructureAllocation* oldASAllocation;
		SVkAPI::AccelerationStructureAllocation compactedASAllocation;
	};

private:
	InstanceList& addBLAS(const Mesh* mesh, VkBuildAccelerationStructureFlagBitsKHR flags);
	void applyCompactedBLASes(VkCommandBuffer cmd); // This will replace any un-compacted BLASes with their compacted counterpart
	void compactBLASes(VkCommandBuffer cmd); // This will compact any previously (finished) built BLAS if it has the compaction flag
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

	// This is filled and clear within the same frame
	std::vector<BlasBuildInfo> m_blasesToBuild;
	// BLASes with the compaction flag are stored in this list on their swap index
	// This allows the compaction function to know when a BLAS is built and ready to be compacted
	std::vector<BlasCompactInfo> m_blasesToCompact;
	// After a compacted AS is created it is stored in this list on their swap index
	// After the swap index is reached, the compacted BLASes replace the non-compacted
	std::vector<std::vector<CompactedBlasInfo>> m_compactedBlases;

	std::vector<std::unordered_map<const Mesh*, InstanceList>> m_bottomInstances; // Each entry in the map is a unique BLAS.
																				  // Each instance inside the instance list is an instance of this BLAS 
																				  // with its own transformation matrix that will be put into the TLAS.
																				  // NOTE: all BLASes are currently multi-buffered, however this is really only needed for DYNAMIC meshes.
																				  //       Consider using a separate map for STATIC meshes without multi-buffering for VRAM gains.
};