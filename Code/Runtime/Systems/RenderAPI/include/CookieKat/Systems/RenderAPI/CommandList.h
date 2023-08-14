#pragma once

#include "CookieKat/Systems/RenderAPI/Pipeline.h"

namespace CKE {
	enum class CommandListType
	{
		Graphics,
		Transfer,
		Compute
	};

	struct CommandListDesc
	{
		CommandListType m_Type;
		bool m_IsPrimary; // TODO: This doesn't really work right now, all the cmd lists are primary
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

	struct TextureRange
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

		TextureRange m_Range;

		u32 m_SrcQueueFamilyIdx;
		u32 m_DstQueueFamilyIdx;
	};

	struct TextureCopyInfo
	{
		TextureHandle     m_TexHandle;
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		TextureLayout     m_Layout;
		u32               m_MipLevel = 0;
		u32               m_BaseArrayLayer = 0;
		u32               m_ArrayLayerCount = 1;
		Int3              m_Offset;
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

#ifdef CKE_GRAPHICS_VULKAN_BACKEND
#include "CookieKat/Systems/RenderAPI/Vulkan/CommandList_Vk.h"
#else
#endif
