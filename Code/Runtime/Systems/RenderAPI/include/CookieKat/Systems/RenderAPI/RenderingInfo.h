#pragma once

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	enum class StoreOp
	{
		Store,
		DontCare,
		None
	};

	enum class LoadOp
	{
		Load,
		Clear,
		DontCare
	};

	struct RenderingAttachment
	{
		TextureViewHandle m_TextureView{};
		TextureLayout     m_Layout{};
		LoadOp            m_LoadOp{};
		StoreOp           m_StoreOp{};
		Vec4              m_ClearValue{0.0f, 0.0f, 0.0f, 0.0f};
	};

	struct RenderingInfo
	{
		UInt2                       m_RenderArea = UInt2{0.0f, 0.0f};
		Vector<RenderingAttachment> m_ColorAttachments{};
		bool                        m_UseDepthAttachment = false;
		RenderingAttachment         m_DepthAttachment;
		bool                        m_UseStencilAttachment = false;
		RenderingAttachment         m_StencilAttachment;
	};
}
