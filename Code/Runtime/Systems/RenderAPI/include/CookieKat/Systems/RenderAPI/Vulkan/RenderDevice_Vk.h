#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Logging/LoggingSystem.h"

#include "CookieKat/Systems/RenderAPI/DescriptorSetBuilder.h"
#include "CookieKat/Systems/RenderAPI/Buffer.h"
#include "CookieKat/Systems/RenderAPI/CommandList.h"
#include "CookieKat/Systems/RenderAPI/Pipeline.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"

#include "CookieKat/Systems/RenderAPI/Internal/RenderResourcesDatabase.h"
#include "CookieKat/Systems/RenderAPI/Internal/Swapchain.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/Instance_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/FrameData.h"

#include <vulkan/vulkan_core.h>

#include "vk_mem_alloc.h"

//-----------------------------------------------------------------------------

// Debugging utilities

static void CheckCallVk(VkResult result, char const* filePath, CKE::i32 fileLine) {
	using namespace CKE;
	if (result != VK_SUCCESS) {
		g_LoggingSystem.Log(LogLevel::Error, LogChannel::Rendering, "Error in vulkan function at path: {}, Line: {}", filePath, fileLine);
	}
}

#define VK_CHECK_CALL(x) CheckCallVk(x, __FILE__, __LINE__)

//-----------------------------------------------------------------------------

namespace CKE {
	class RenderDevice
	{
	public:
		// Lifetime 
		//-----------------------------------------------------------------------------

		// Used to pass a pointer to the platform window (win32, not GLFW)
		// WARNING: Must be called before calling Initialize()
		void SetRenderTargetData(void* pData);

		// A RenderDevice must be initialized using RenderDevice::Initialize(...)
		// and must be destroyed calling RenderDevice::Shutdown()
		void Initialize(Int2 backBufferSize);
		void Shutdown();

		// Create the device SwapChain with the given size and the
		// supported formats found in the device. The specific configuration
		// of the SwapChain is inside this method
		void CreateSwapChain(Int2 frameBufferSize, HWND window);

		// Frame Sync
		//-----------------------------------------------------------------------------

		// May block the thread to wait for the next available BackBuffer
		void AcquireNextBackBuffer();

		// Submits a SwapChain flip to the present queue and resets
		// some of the per-frame internal data.
		// IMPORTANT: The user must correctly signal and wait on the frame sync points
		void Present();

		// Returns the semaphore that is signaled when an image is available to render
		SemaphoreHandle GetImageAvailableSemaphore();

		// Returns the semaphore that should be signaled when a
		// render has finished writing to the backbuffer
		SemaphoreHandle GetRenderFinishedSemaphore();
		FenceHandle     GetInFlightFence();

		// Blocks the calling thread until the Render Device is idle
		// In general, it should only be used for debugging purposes
		void WaitForDevice();

		// Buffers
		//-----------------------------------------------------------------------------

		// Creates a buffer using the given description and returns its handle
		BufferHandle CreateBuffer(BufferDesc bufferDesc);

		// Destroys a buffer associated with the given handle
		void DestroyBuffer(BufferHandle bufferHandle);

		void* MapBuffer(BufferHandle bufferHandle);

		void UnMapBuffer(BufferHandle bufferHandle);

		// Returns a pointer to the mapped memory space of the given buffer
		//
		// Pre-Requisites:
		//   The buffer must be mapped
		void* GetBufferMappedPtr(BufferHandle bufferHandle);

		// Textures and Samplers
		//-----------------------------------------------------------------------------

		// Create a texture with the given description
		TextureHandle CreateTexture(TextureDesc desc);

		// Create a texture view with the given description
		TextureViewHandle CreateTextureView(TextureViewDesc desc);

		// Create a sampler with the given description
		SamplerHandle CreateSampler(SamplerDesc desc);

		// Destroys the texture and all of views
		//
		// Pre-Condition:
		//	 The texture and its views should not be in use.
		void DestroyTexture(TextureHandle textureHandle);

		// Destroys the given texture view.
		//
		// Pre-Condition:
		//	 View should not be in use.
		void DestroyTextureView(TextureViewHandle handle);

		// Destroys the given sampler.
		//
		// Pre-Condition:
		//	 Sampler should not be in use.
		void DestroySampler(SamplerHandle samplerHandle);

		// Pipelines
		//-----------------------------------------------------------------------------

		// Creates a pipeline layout using the given description
		PipelineLayoutHandle CreatePipelineLayout(PipelineLayoutDesc const& layoutDesc);

		// Creates a pipeline layout using the given description
		//
		// Pre-Condition:
		//	 Pipeline should not be in use.
		void DestroyPipelineLayout(PipelineLayoutHandle handle);

		// Create a graphics pipeline using the given description.
		PipelineHandle CreateGraphicsPipeline(GraphicsPipelineDesc const& desc);

		// Create a compute pipeline using the given description.
		PipelineHandle CreateComputePipeline(ComputePipelineDesc const& desc);

		// Destroy the given pipeline.
		//
		// Pre-Condition:
		//	 Pipeline should not be in use.
		void DestroyPipeline(PipelineHandle pipelineHandle);

		// Semaphores and Fences
		//-----------------------------------------------------------------------------

		// Create a semaphore for GPU-GPU Synchronization
		SemaphoreHandle CreateSemaphoreGPU();

		// Destroy the given semaphore
		//
		// Pre-Condition:
		//	 Fence should not be in use.
		void DestroySemaphore(SemaphoreHandle handle);

		// Create a fence used for GPU-CPU synchronization
		FenceHandle CreateFence(bool createSignaled);

		// Destroy the given fence.
		//
		// Pre-Condition:
		//	 Fence should not be in use.
		void DestroyFence(FenceHandle fence);

		// Block the calling thread until the given fence is signaled
		void WaitForFence(FenceHandle fence);

		// Reset the given fence so it can be signaled again
		void ResetFence(FenceHandle fence);

		// Queues
		//-----------------------------------------------------------------------------

		CommandQueueHandle CreateCommandQueue(CommandQueueDesc desc);
		void               SubmitCommandList(CommandList* cmdList, u32 cmdListCount, CmdListSubmitInfo submitInfo);

		// Command Lists
		//-----------------------------------------------------------------------------

		// Returns an available graphics command list for this frame
		//
		// Asserts:
		//   Requested cmdList count is lower than the max amount
		CommandList GetGraphicsCmdList();

		// Submit a command list to the graphics queue
		void SubmitGraphicsCommandList(CommandList& cmdList, CmdListSubmitInfo submitInfo);

		// Submit a batch of command lists to the graphics queue
		void SubmitGraphicsCommandLists(Vector<CommandList>& cmdList, CmdListSubmitInfo submitInfo);

		// Returns the indices of the selected GPU queues
		CommandQueueFamilyIndices GetQueueFamilyIndices();

		// Descriptor Sets
		//-----------------------------------------------------------------------------

		// Returns an interface object used to define a descriptor set that can be bound
		// to a pipeline to access data from shaders.
		// NOTE: Don't create a descriptor set builder directly, use this method instead.
		DescriptorSetBuilder CreateDescriptorSetBuilder(PipelineHandle p, u64 setIndex);

		// Utils
		//-----------------------------------------------------------------------------

		// The frame Idx is a incrementing and repeating number that goes
		// from 0 to MAX_FRAMES_IN_FLIGHT.
		//
		// It is mainly used to identify and access per-frame-in-flight data
		// such as objects, lights data, semaphores...
		u32 GetFrameIdx();

		// Returns the SwapChain texture of the current frame
		TextureHandle GetBackBuffer();

		// Returns the SwapChain view of the current frame
		TextureViewHandle GetBackBufferView();

		// Returns a description of the SwapChain texture
		TextureDesc GetBackBufferDesc();

		// Returns the SwapChain/BackBuffer size as a UInt2
		UInt2 GetBackBufferSize() const;

		// Returns the SwapChain/BackBuffer size as a UInt3
		UInt3 GetBackBufferSize3() const;

		// Records a deferred backbuffer resize event, it will be handled
		// after the current frame is presented
		void RecordBackBufferResized(Int2 newSize);

	private:
		// SwapChain
		//-----------------------------------------------------------------------------

		// Destroys the SwapChain texture views
		void DestroySwapChainViews();

		// Destroys and recreates the SwapChain/BackBuffers, called in the case of a
		// window resize event
		void RecreateSwapChain(Int2 newSize);
		void PrepareDescriptorPoolsAndSetsForPipeline(Vector<ShaderBinding> const& shaderBindings, Pipeline& pipeline,
		                                              PipelineLayout*              pLayout);

		// Create a shader module using the given bytecode
		VkShaderModule CreateShaderModule(Vector<u8> const& code);

		// Commands
		//-----------------------------------------------------------------------------

		// Internal Use Only thats why we don't define it here
		template <typename T>
		void SubmitTCommandLists(Vector<T>& cmdList, CmdListSubmitInfo submitInfo, VkQueue queue);

		// Submit a list of command buffers to a given queue
		void SubmitCommandBuffers(Vector<VkCommandBuffer> const& cmdBuffers, VkQueue queue, CmdListSubmitInfo submitInfo);

		// Create all of the command pools that will be used
		void CreateDefaultCommandPools();

		// Destroy all of the command pools that were created for the device
		void DestroyDefaultCommandPools();

		// We generate a default amount of command lists
		// that can be queried every frame and be reused
		// NOTE: See render settings for specific amounts
		void CreateDefaultCommandLists();

		//Create an internal per-frame command list using the given description
		void CreateCommandList(CommandListDesc desc);
		void SetObjectDebugName(u64 objectHandle, VkObjectType objectType, const char* name);

		// Auxiliary
		//-----------------------------------------------------------------------------

		void ResetAllPerFrameData();

		// Auxiliary function to create a buffer
		void CreateBuffer_Internal(BufferDesc bufferDesc, Buffer* pBuffer);

		// Copy any data to a buffer in CPU-GPU or GPU-Only memory.
		// TODO: The implementation is a bit sketchy but it works for now
		void CopyDataToBuffer(BufferDesc bufferDesc, void* pInitialData, u32 dataSizeInBytes, Buffer& buffer);

		// Returns a suitable memory type for the given requirements
		u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

		// Just a shortcut to retrieve the current frame data structure
		void GetCurrentFrameData();

		void ConvertQueueFamilyFlagsToIndices(QueueFamilyFlags familyFlags, Array<u32, 3>& familyIndices, i32& familyIndicesCount);

		// Descriptors
		//-----------------------------------------------------------------------------

		void CreatePipelineDescriptorPool(Pipeline& pipeline, Vector<Vector<ShaderBinding>> const& sortedBindings);
		void CreateDescriptorSetLayouts(Vector<Vector<ShaderBinding>> const& sortedBindings);

		// Allocate a descriptor set for the given pipeline using its descriptor pool
		void AllocateDescriptorSet(PipelineHandle pipelineHandle, PipelineLayout* pPipelineLayout, u32 setIndex, u32 countPerFrame);
		DescriptorSetHandle CreateDescriptorSetForFrame(PipelineHandle pipelineHandle, u32 layoutSlot,
		                                                Vector<DescriptorSetBuilder::Bindings>& shaderBindings);

	private:
		friend RenderInstance;
		friend DescriptorSetBuilder;
		friend class RenderDeviceDebugUtils;

	private:
		RenderInstance* m_RenderInstance{};

		RenderResourcesDatabase m_ResourcesDB{};

		CommandBufferManager m_CommandBufferManager{};
		FrameSyncObjects     m_FrameSyncObjects{};
		u32                  m_CurrFrameInFlightIdx = 0;

		VkPhysicalDevice m_PhysicalDevice{};
		VkDevice         m_Device{};
		void*            m_pRenderTargetData = nullptr; // Pointer to platform window

		CommandQueueFamilyIndices m_QueueFamilyIndices{};
		VmaAllocator              m_Allocator;
	};
}

namespace CKE {
	// Auxiliary class used to debug the render device state
	class RenderDeviceDebugUtils
	{
	public:
		void Initialize(RenderDevice* pDevice);
		void PrintAllResourcesState();

	private:
		RenderDevice* m_pDevice = nullptr;
	};
}
