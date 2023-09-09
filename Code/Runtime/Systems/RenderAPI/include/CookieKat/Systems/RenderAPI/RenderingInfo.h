#pragma once

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	// Rendering Attachment Store Operation
	enum class StoreOp : u8
	{
		Store,
		DontCare,
		None
	};

	// Rendering Attachment Load Operation
	enum class LoadOp : u8
	{
		Load,
		Clear,
		DontCare
	};

	// Single Rendering Attachment Info
	struct RenderingAttachment
	{
		TextureViewHandle m_TextureView = TextureViewHandle::Invalid();
		TextureLayout     m_Layout = TextureLayout::Undefined;
		LoadOp            m_LoadOp = LoadOp::DontCare;
		StoreOp           m_StoreOp = StoreOp::DontCare;
		Vec4              m_ClearValue{0.0f, 0.0f, 0.0f, 0.0f};
	};

	struct RenderingInfo
	{
		UInt2                       m_RenderArea = UInt2{0.0f, 0.0f};
		Vector<RenderingAttachment> m_ColorAttachments{};
		bool                        m_UseDepthAttachment = false;
		RenderingAttachment         m_DepthAttachment{};
		bool                        m_UseStencilAttachment = false;
		RenderingAttachment         m_StencilAttachment{};
	};
}
