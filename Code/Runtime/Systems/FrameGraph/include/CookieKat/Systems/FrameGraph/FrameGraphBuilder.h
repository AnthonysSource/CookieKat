#pragma once

#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"
#include "CookieKat/Systems/RenderAPI/Buffer.h"

namespace CKE {
	struct FGTextureDesc
	{
		TextureFormat     m_Format = TextureFormat::R8G8B8A8_SRGB;
		bool              m_UseSizeRelativeToRenderTarget = false;
		UInt3             m_Size = UInt3{1, 1, 1};
		Vec2              m_RelativeSize{1.0f, 1.0f};
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		TextureType       m_TextureType = TextureType::Tex2D;
		TextureUsage      m_Usage = TextureUsage::Transfer_Dst | TextureUsage::Sampled;
		u32               m_ArraySize = 1;
		u32               m_MipLevels = 1;
		bool              m_IsCubeMapCompatible = false;
		DebugString       m_Name{};

		FGTextureDesc() = default;

		FGTextureDesc(TextureFormat format, UInt3             size, TextureAspectMask aspectMask,
		              TextureType   textureType, TextureUsage usage)
			: m_Format{format},
			  m_Size{size},
			  m_AspectMask{aspectMask},
			  m_TextureType{textureType},
			  m_Usage{usage} {}

		FGTextureDesc(DebugString       name, TextureFormat format, UInt3 size,
		              TextureAspectMask aspectMask,
		              TextureType       textureType, TextureUsage usage)
			: m_Format{format},
			  m_Size{size},
			  m_AspectMask{aspectMask},
			  m_TextureType{textureType},
			  m_Usage{usage},
			  m_Name{name} {}
	};

	// Passed to the FG Passes to declare their resource usage
	class FrameGraphSetupContext
	{
	public:
		void CreateTransientTexture(FGResourceID         id, TextureDesc desc,
		                            TextureExtraSettings extraSettings);
		void CreateTransientBuffer(FGResourceID id, BufferDesc desc);

		void UseTexture(FGResourceID id, FGPipelineAccessInfo accessInfo);
		void UseBuffer(FGResourceID id);

	private:
		FGTextureAccessInfo GetTexAccessInfo(FGResourceID texID);

	private:
		friend class FrameGraph;

		Vector<FGPassResourceUsageInfo> m_ResourceUsageMetadata;
		Vector<FGTextureAccessInfo>     m_TextureAccessInfo;

		// Resource Creation Data
		//-----------------------------------------------------------------------------

		struct FGTexCreateInfo
		{
			FGResourceID         m_ID;
			TextureDesc          m_Desc;
			TextureExtraSettings m_Extra;
		};

		struct FGBuffCreateInfo
		{
			FGResourceID m_ID;
			BufferDesc   m_Desc;
		};

		Vector<FGTexCreateInfo>  m_CreateTextures;
		Vector<FGBuffCreateInfo> m_CreateBuffers;
	};
}
