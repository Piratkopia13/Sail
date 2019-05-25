#pragma once

// Link necessary d3d12 libraries
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

// Include the minimal needed from windows.h
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_6.h> // Only used for initialization of the device and swap chain
#include <d3dcompiler.h>
#ifdef _DEBUG
#include <initguid.h>
#include <DXGIDebug.h>
#endif
#include "Sail/api/GraphicsAPI.h"

// Forward declarations
class Win32Window;

template <typename T>
using wComPtr = Microsoft::WRL::ComPtr<T>;

namespace GlobalRootParam {
	enum Slot {
		CBV_TRANSFORM = 0,
		CBV_DIFFUSE_TINT,
		CBV_CAMERA,
		DT_SRVS,
		DT_SAMPLERS,
		SIZE
	};
}

class DX12API : public GraphicsAPI {
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

	ID3D12Device5* getDevice() const;
	ID3D12RootSignature* getGlobalRootSignature() const;

	inline UINT getFrameIndex() const;

	void renderToBackBuffer() const;
	// TODO: replace with event
	void resize(UINT width, UINT height);
private:
	void createDevice();
	void createCmdInterfacesAndSwapChain(Win32Window* window);
	void createFenceAndEventHandle();
	void createRenderTargets();
	void createGlobalRootSignature();
	//void createShaderResources();
	void createDepthStencilResources(Win32Window* window);
	void nextFrame();

	void resizeBuffers(UINT width, UINT height);


private:
	static const UINT NUM_SWAP_BUFFERS;

	UINT m_backBufferIndex;

	wComPtr<ID3D12Device5> m_device;
#ifdef _DEBUG
	wComPtr<IDXGIFactory2> m_dxgiFactory;
#endif
	// Only used for initialization
	IDXGIFactory7* m_factory;
	IDXGIAdapter3* m_adapter3;

	// Queues
	wComPtr<ID3D12CommandQueue> m_directCommandQueue;
	wComPtr<ID3D12CommandQueue> m_computeCommandQueue;
	wComPtr<ID3D12CommandQueue> m_copyCommandQueue;

	// Commands
	struct Command {
		std::vector<wComPtr<ID3D12CommandAllocator>> allocators; // Allocator only grows, use multple (one for each thing)
		wComPtr<ID3D12GraphicsCommandList4> list;
	};
	Command m_preCommand;
	Command m_postCommand;
	Command m_copyCommand;
	Command m_computeCommand;

	wComPtr<ID3D12DescriptorHeap> m_renderTargetsHeap;
	wComPtr<IDXGISwapChain4> m_swapChain;
	std::vector<wComPtr<ID3D12Resource1>> m_renderTargets;
	wComPtr<ID3D12RootSignature> m_globalRootSignature;

	// Fences
	// TODO: check which ones are needed
	std::vector<UINT64> m_fenceValues;
	wComPtr<ID3D12Fence1> m_fence;
	wComPtr<ID3D12Fence1> m_computeQueueFence;
	wComPtr<ID3D12Fence1> m_copyQueueFence;
	wComPtr<ID3D12Fence1> m_directQueueFence;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	HANDLE m_eventHandle;
	UINT m_renderTargetDescriptorSize;

	// Depth/Stencil
	wComPtr<ID3D12Resource> m_depthStencilBuffer;
	wComPtr<ID3D12DescriptorHeap> m_dsDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_dsvDescHandle;

};