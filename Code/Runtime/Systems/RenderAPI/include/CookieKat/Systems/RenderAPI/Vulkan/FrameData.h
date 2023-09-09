#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CommandList.h"
#include "RenderHandle.h"
#include "RenderResources_Vk.h"

#include <vulkan/vulkan_core.h>

namespace CKE {
	// Per-frame data associated to a pipeline
	struct PipelineFrameData
	{
		VkDescriptorPool            m_DescriptorPool{};
		Vector<DescriptorSetHandle> m_DescriptorSets{};
	};

	class FrameResources
	{
	public:
		void Initialize(RenderDevice& device);
		void ResetForNewFrame();
		void Destroy(RenderDevice& device);

	public:
		Map<BufferHandle, Buffer>               m_Buffers{};
		Map<DescriptorSetHandle, DescriptorSet> m_DescriptorSets{};
		Map<PipelineHandle, PipelineFrameData>  m_PipelineFrameData{};
	};

	class CommandBufferManager
	{
	public:
		VkCommandBuffer GetNextCmdBuffer(QueueType type);
		VkCommandBuffer GetNextGraphics();
		VkCommandBuffer GetNextTransfer();
		VkCommandBuffer GetNextCompute();

	private:
		u64                     m_LastCmdIdxGraphics = 0;
		u64                     m_LastCmdIdxTransfer = 0;
		u64                     m_LastCmdIdxCompute = 0;
		Vector<VkCommandBuffer> m_GraphicsCommandBuffer{};
		Vector<VkCommandBuffer> m_TransferCommandBuffer{};
		Vector<VkCommandBuffer> m_ComputeCommandBuffer{};

		VkCommandPool m_GraphicsCommandPool{};
		VkCommandPool m_TransferCommandPool{};
		VkCommandPool m_ComputeCommandPool{};
	};

	class FrameSyncObjects
	{
	public:
		// Syncing Data
		inline SemaphoreHandle GetImageAvailableSemaphore() { return m_ImageAvailableSemaphore[m_FrameIdx]; }
		inline SemaphoreHandle GetRenderFinishedSemaphore() { return m_RenderFinishedSemaphore[m_FrameIdx]; }
		inline FenceHandle     GetInFlightFence() { return m_InFlightFence[m_FrameIdx]; }

		void UpdateFrameIdx(u32 newIdx) {
			CKE_ASSERT(newIdx < RenderSettings::MAX_FRAMES_IN_FLIGHT);
			m_FrameIdx = newIdx;
		}

	private:
		u32 m_FrameIdx = 0;
		FrameArray<SemaphoreHandle> m_ImageAvailableSemaphore{};
		FrameArray<SemaphoreHandle> m_RenderFinishedSemaphore{};
		FrameArray<FenceHandle> m_InFlightFence{};
	};
}
