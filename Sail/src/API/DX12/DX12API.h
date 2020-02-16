#pragma once

// Link necessary d3d12 libraries
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

#include <windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_6.h> // Only used for initialization of the device and swap chain
#include <d3dcompiler.h>
#ifdef _DEBUG
#include <initguid.h>
#include <DXGIDebug.h>
#include <DXProgrammableCapture.h>
#endif
#include "Sail/api/GraphicsAPI.h"
#include <map>

// Forward declarations
class Win32Window;
class DescriptorHeap;

template <typename T>
using wComPtr = Microsoft::WRL::ComPtr<T>;

namespace GlobalRootParam {
	enum Slot {
		CBV_0 = 0,
		CBV_1,
		CBV_2,
		DT_SRV_0TO9_UAV_10TO20,
		SRV_10,
		SRV_11,
		UAV_0,
		UAV_1,
		SIZE
	};
}

class DX12API : public GraphicsAPI {
public:
	static const UINT NUM_SWAP_BUFFERS;
	static const UINT NUM_GPU_BUFFERS;

	struct Command {
		std::vector<wComPtr<ID3D12CommandAllocator>> allocators; // Allocator only grows, use multiple (one for each thing)
		wComPtr<ID3D12GraphicsCommandList4> list;
	};

	class CommandQueue {
	public:
		CommandQueue(DX12API* context, D3D12_COMMAND_LIST_TYPE type, LPCWSTR name = L"Unnamed Command Queue");
		UINT64 signal();
		void wait(UINT64 fenceValue) const;
		bool waitOnCPU(UINT64 fenceValue, HANDLE eventHandle) const;
		ID3D12CommandQueue* get() const;
		UINT64 getCurrentFenceValue() const;
		UINT64 getCompletedFenceValue() const;
		void reset();
		void scheduleSignal(std::function<void(UINT64)> func);
		void executeCommandLists(std::initializer_list<ID3D12CommandList*> cmdLists);
		void executeCommandLists(ID3D12CommandList* const* cmdLists, const int nLists);
	private:
		DX12API* m_context;
		wComPtr<ID3D12CommandQueue> m_commandQueue;
		static UINT64 sFenceValue;
		static wComPtr<ID3D12Fence1> sFence;
		std::vector<std::function<void(UINT64)>> m_queuedSignals;
	};

	// Helper method to create resources that require one for each swap buffer
	template <typename T>
	std::vector<T> createFrameResource() {
		auto resource = std::vector<T>();
		resource.resize(NUM_GPU_BUFFERS);
		return resource;
	}
	// Retrieves the resource for the current back buffer index
	template <typename T>
	T getFrameResource(const std::vector<T>& resource) {
		return resource[m_context->getSwapIndex()];
	}

public:
	DX12API();
	~DX12API();

	virtual bool init(Window* window) override;
	virtual void clear(const glm::vec4& color) override;
	virtual void setDepthMask(DepthMask setting) override;
	virtual void setFaceCulling(Culling setting) override;
	virtual void setBlending(Blending setting) override;
	virtual void present(bool vsync = false) override;
	virtual unsigned int getMemoryUsage() const override;
	virtual unsigned int getMemoryBudget() const override;
	virtual void toggleFullscreen() override;
	virtual bool onResize(WindowResizeEvent& event) override;
	virtual void waitForGPU() override;

	struct RootSignEntry {
		UINT rootSigIndex;
		UINT dtOffset;
	};
	RootSignEntry getRootSignEntryFromRegister(const std::string& reg) const;

	ID3D12Device5* getDevice() const;
	ID3D12RootSignature* getGlobalRootSignature() const;
	UINT getSwapIndex() const; // Returns 0 or 1
	UINT getFrameIndex() const; // Returns 0, 1, ... NUM_SWAP_BUFFERS
	UINT getNumGPUBuffers() const; // Always returns 2 - as no more than two buffers are needed for any gpu based resource

	DescriptorHeap* const getMainGPUDescriptorHeap() const;
	const D3D12_CPU_DESCRIPTOR_HANDLE& getCurrentRenderTargetCDH() const;
	ID3D12Resource* getCurrentRenderTargetResource();
	const D3D12_CPU_DESCRIPTOR_HANDLE& getDsvCDH() const;
	const D3D12_CPU_DESCRIPTOR_HANDLE& getDepthStencilViewCDH() const;
	IDXGISwapChain4* const getSwapChain() const;
	const D3D12_VIEWPORT* getViewport() const;
	const D3D12_RECT* getScissorRect() const;
	CommandQueue* getDirectQueue() const;
	unsigned int getFrameCount() const; // Returns the number of elapsed frames

#ifdef _DEBUG
	void beginPIXCapture() const;
	void endPIXCapture() const;
#endif

	// Schedules a resource for initialization
	void scheduleResourceForInit(std::function<bool(ID3D12GraphicsCommandList4*)> initFunc);
	// Initializes all resources previously scheduled using scheduleResourceForInit(..)
	// It is up to the renderer to call this method
	void initResources(ID3D12GraphicsCommandList4* cmdList);

	void initCommand(Command& cmd, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT, LPCWSTR name = L"Unnamed Commmand list or allocator object");
	void renderToBackBuffer(ID3D12GraphicsCommandList4* cmdList) const;
	void prepareToRender(ID3D12GraphicsCommandList4* cmdList) const;
	void prepareToPresent(ID3D12GraphicsCommandList4* cmdList) const;

private:
	void createDevice();
	void createCmdInterfacesAndSwapChain(Win32Window* window);
	void createEventHandle();
	void createRenderTargets();
	void createGlobalRootSignature();
	void createShaderResources();
	void createDepthStencilResources(Win32Window* window);
	void createViewportAndScissorRect(Win32Window* window);

	void nextFrame();

	void resizeBuffers(UINT width, UINT height);

private:
	// Whether or not tearing is available for fullscreen borderless windowed mode.
	bool m_tearingSupport;
	bool m_windowedMode;
	RECT m_windowRect;
	
	UINT m_backBufferIndex; // 0, 1, .. numSwapBuffers
	UINT m_swapIndex; // 0 or 1
	D3D12_CPU_DESCRIPTOR_HANDLE m_currentRenderTargetCDH;
	ID3D12Resource* m_currentRenderTargetResource;
	float m_clearColor[4];
	unsigned int m_frameCount;

	wComPtr<ID3D12Device5> m_device;
#ifdef _DEBUG
	wComPtr<IDXGIFactory2> m_dxgiFactory;
	wComPtr<IDXGraphicsAnalysis> m_pixGa;
#endif
	// Only used for initialization
	IDXGIFactory7* m_factory;
	IDXGIAdapter3* m_adapter3;

	std::vector<std::function<bool(ID3D12GraphicsCommandList4*)>> m_resourcesScheduledForInit;

	// Queues
	std::unique_ptr<CommandQueue> m_directCommandQueue;
	UINT64 m_directQueueFenceValues[2];

	std::unique_ptr<DescriptorHeap> m_cbvSrvUavDescriptorHeap;

	wComPtr<ID3D12DescriptorHeap> m_renderTargetsHeap;
	wComPtr<IDXGISwapChain4> m_swapChain;
	std::vector<wComPtr<ID3D12Resource1>> m_renderTargets;
	wComPtr<ID3D12RootSignature> m_globalRootSignature;
	// Map slot name to signature index and dt offsets
	std::unordered_map<std::string, RootSignEntry> m_globalRootSignatureRegisters;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	HANDLE m_eventHandle;
	UINT m_renderTargetDescriptorSize;

	// Depth/Stencil
	wComPtr<ID3D12Resource> m_depthStencilBuffer;
	wComPtr<ID3D12DescriptorHeap> m_dsDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvDescHandle;

};