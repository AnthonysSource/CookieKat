#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CommandList.h"
#include "RenderHandle.h"

#include <vulkan/vulkan_core.h>

namespace CKE {
	class FrameResources
	{
	public:
		Map<BufferHandle, Buffer>               m_Buffers{};
		Map<DescriptorSetHandle, DescriptorSet> m_DescriptorSets{};
	};

	// Per-frame data associated to a pipeline
	struct PipelineFrameData_Vk
	{
		VkDescriptorPool            m_DescriptorPool{};
		Vector<DescriptorSetHandle> m_DescriptorSets{};
	};

	// Per-frame data managed by the render device
	class FrameData_Vk
	{
	public:
		void Initialize(RenderDevice& device);
		void ResetForNewFrame();
		void Destroy(RenderDevice& device);

		// Syncing Data
		inline SemaphoreHandle GetImageAvailableSemaphore() { return m_ImageAvailableSemaphore; }
		inline SemaphoreHandle GetRenderFinishedSemaphore() { return m_RenderFinishedSemaphore; }
		inline FenceHandle     GetInFlightFence() { return m_InFlightFence; }

		// Command Lists
		VkCommandBuffer GetNextCmdBuffer(CommandListType type);
		VkCommandBuffer GetNextGraphicsCmdBuffer();
		VkCommandBuffer GetNextTransferCmdBuffer();
		VkCommandBuffer GetNextComputeCmdBuffer();

		// Pipeline Bindings
		inline PipelineFrameData_Vk& GetPipelineState(PipelineHandle p) { return m_PipelineFrameData[p]; }

	public:
		SemaphoreHandle m_ImageAvailableSemaphore{};
		SemaphoreHandle m_RenderFinishedSemaphore{};
		FenceHandle     m_InFlightFence{};

		u64                     m_LastCmdIdxGraphics = 0;
		u64                     m_LastCmdIdxTransfer = 0;
		u64                     m_LastCmdIdxCompute = 0;
		Vector<VkCommandBuffer> m_GraphicsCommandBuffer{};
		Vector<VkCommandBuffer> m_TransferCommandBuffer{};
		Vector<VkCommandBuffer> m_ComputeCommandBuffer{};

		VkCommandPool m_GraphicsCommandPool{};
		VkCommandPool m_TransferCommandPool{};
		VkCommandPool m_ComputeCommandPool{};

		Map<PipelineHandle, PipelineFrameData_Vk> m_PipelineFrameData{};
	};
}
