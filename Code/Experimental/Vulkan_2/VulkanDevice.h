#pragma once

#include "Auxiliary.h"
#include "Context.h"

class VulkanBackend;

using namespace CKE;

class Buffer
{
public:
	u32 GetStride();
	u64 GetSize();
	u32 GetNumElements();

	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;

	u64 m_Size;
};

struct Pipeline
{
	VkPipeline m_Pipeline;
	VkPipelineLayout m_Layout;
};

struct SwapchainInitialConfig
{
	// SwapChain Config
	static VkSurfaceFormatKHR SelectSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR SelectSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
};

class VulkanDevice
{
public:
	void Initialize();
	void Shutdown();

	u32 AcquireNextImage();
	void Present();

	void WaitIdle();

	// Pipeline
	//-----------------------------------------------------------------------------

	Pipeline CreatePipeline();
	void DestroyPipeline(Pipeline pipeline);

	// Command Recording
	//-----------------------------------------------------------------------------

	GraphicsContext GetGraphicsContext();
	TransferContext GetTransferContext();
	void SubmitGraphicsWork(GraphicsContext& context);
	void SubmitTransferWork(TransferContext& context);

	// Buffers
	//-----------------------------------------------------------------------------

	Buffer CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, int memoryProperties);
	void DestroyBuffer(Buffer& buffer);

	void CreateUniformBuffer(u64 uboSize);

	// Textures
	//-----------------------------------------------------------------------------

	VkImage CreateTexture(u8* textureData, u32 width, u32 height);

	// Memory
	//-----------------------------------------------------------------------------

	void MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void** data);
	void UnmapMemory(VkDeviceMemory memory);

	void TriggerFrameBufferResized();

	VkExtent2D GetSwapChainExtent() const { return m_SwapChain.m_Extent; }

private:

	// SwapChain
	//-----------------------------------------------------------------------------

	void CreateSwapChain();
	void CreateSwapChainImageViews(VkDevice device);
	void RecreateSwapChain(VkDevice device);
	void DestroySwapChain();

	VkShaderModule CreateShaderModule(Vector<u8> const& code);

	// Render Pass & FrameBuffer
	//-----------------------------------------------------------------------------

	void CreateRenderPass();
	void CreateFrameBufferForRenderPass();

	void DestroyRenderPass();

	// Command Pools
	//-----------------------------------------------------------------------------

	void CreateGraphicsCommandPool();
	void CreateTransferCommandPool();

	void CreateGraphicsCommandBuffers();
	void CreateTransferCommandBuffers();

	void DestroyAllCommandPools();

	// Descriptors
	void CreateDescriptorPool();
	void DestroyDescriptorPool();

	void CreateDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	void CreateDescriptorSets();

	// Memory
	//-----------------------------------------------------------------------------

	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);

	// Default Frame Syncing
	//-----------------------------------------------------------------------------

	void CreateFrameSyncObjects();
	void DestroyFrameSyncObjects();

private:
	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice m_Device{};

	SwapChainData m_SwapChain{};
	u32 m_SwapchainIdx;

	// Queues + Commands
	//-----------------------------------------------------------------------------

	QueueFamilyIndices m_FamilyIndices{};

	VkQueue m_GraphicsQueue{};
	VkQueue m_TransferQueue{};
	VkQueue m_PresentQueue{};

	VkCommandPool m_GraphicsCommandPool{};
	VkCommandPool m_TransferCommandPool{};

	VkDescriptorPool m_DescriptorPool{};

	// Render Pass + Pipeline
	//-----------------------------------------------------------------------------

	VkRenderPass m_RenderPass{};
	VkDescriptorSetLayout m_DescriptorSetLayout{};

	VkPipelineLayout m_PipelineLayout{};
	VkPipeline m_GraphicsPipeline{};

	// Frame Sync Primitives
	//-----------------------------------------------------------------------------

	FrameData m_Frame{};

private:
	friend VulkanBackend;
	VulkanBackend* m_Backend;
};
