#pragma once

#include "CookieKat/Systems/RenderAPI/Pipeline.h"
#include "CookieKat/Systems/RenderAPI/CommandQueue.h"

namespace CKE {
	struct CommandListDesc
	{
		QueueType m_Type;
		bool      m_IsPrimary; // TODO: This doesn't really work right now, all the cmd lists are primary
	};

	struct CmdListWaitSemaphoreInfo
	{
		SemaphoreHandle m_Semaphore;
		PipelineStage   m_Stage;
	};

	struct CmdListSubmitInfo
	{
		Vector<CmdListWaitSemaphoreInfo> m_WaitSemaphores;
		Vector<SemaphoreHandle>          m_SignalSemaphores;

		FenceHandle m_SignalFence;
	};
}

namespace CKE {
	struct TextureSubresourceRange
	{
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		u32               m_BaseMip = 0;
		u32               m_MipCount = 1;
		u32               m_BaseLayer = 0;
		u32               m_LayerCount = 1;
	};

	struct TextureBarrierDescription
	{
		PipelineStage m_SrcStage;
		AccessMask    m_SrcAccessMask;
		PipelineStage m_DstStage;
		AccessMask    m_DstAccessMask;

		TextureLayout m_OldLayout;
		TextureLayout m_NewLayout;

		TextureHandle     m_Texture;
		TextureAspectMask m_AspectMask;

		TextureSubresourceRange m_Range;

		u32 m_SrcQueueFamilyIdx;
		u32 m_DstQueueFamilyIdx;
	};

	struct TextureSubresourceLayers
	{
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		u32               m_MipLevel = 0;
		u32               m_ArrayBaseLayer = 0;
		u32               m_ArrayLayerCount = 1;
	};

	struct BufferCopyInfo
	{
		u64 srcOffset = 0;
		u64 dstOffset = 0;
		u64 size = 0;

		BufferCopyInfo() = default;
	};

	enum class IndicesFormat
	{
		UINT16 = 0,
		UINT32 = 1,
	};

	enum class PipelineBindPoint
	{
		Graphics,
		Compute,
		RayTracing
	};

	struct BufferImageCopyInfo
	{
		u64                      bufferOffset;
		u32                      bufferRowLength;
		u32                      bufferImageHeight;
		TextureSubresourceLayers imageSubresource;
		Int3                     imageOffset;
		UInt3                    imageExtent;
	};

	struct TextureCopyInfo
	{
		TextureHandle            m_TexHandle{};
		TextureLayout            m_Layout{};
		TextureSubresourceLayers m_Subresource{};
		Int3                     m_Offset;
	};

	struct ImageSubresourceLayers
	{
		TextureAspectMask m_AspectMask;
		u32               m_MipLevel;
		u32               m_BaseLayer;
		u32               m_LayerCount;
	};

	struct BufferImageCopy
	{
		u64                    m_BufferOffset;
		u32                    m_BufferRowLength;
		u32                    m_BufferImageHeight;
		ImageSubresourceLayers m_ImageSubresource;
		Vec3                   m_ImageOffset;
		Vec3                   m_ImageExtent;
	};
}

namespace CKE {

	struct QueuePriority
	{
		QueuePriority(f32 value) : m_Value{ value } {}

		inline static QueuePriority Low() { return QueuePriority{ 0.1f }; }
		inline static QueuePriority Medium() { return QueuePriority{ 0.5f }; }
		inline static QueuePriority High() { return QueuePriority{ 0.9f }; }
		inline static QueuePriority Max() { return QueuePriority{ 1.0f }; }

		f32 m_Value = 0.0f;
	};

	struct CommandQueueDesc
	{
		QueueType m_QueueType = QueueType::Graphics;
		QueuePriority m_Priority = QueuePriority::Medium();
	};
}

#ifdef CKE_GRAPHICS_VULKAN_BACKEND
#include "CookieKat/Systems/RenderAPI/Vulkan/CommandList_Vk.h"
#else
#endif
