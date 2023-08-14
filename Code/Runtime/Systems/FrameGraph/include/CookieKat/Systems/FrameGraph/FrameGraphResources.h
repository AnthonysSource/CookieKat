#pragma once

#include "CookieKat/Systems/RenderAPI/Buffer.h"
#include "CookieKat/Systems/RenderAPI/Pipeline.h"

namespace CKE {
	// TODO: Create Debug and Release versions of these IDs
	using FGResourceID = String;
	using FGRenderPassID = String;

	// Where does the resource come from
	enum class FGResourceSource
	{
		Transient,
		Imported,
	};

	// Type of the GPU resource
	enum class FGResourceType
	{
		Texture,
		Buffer,
	};

	// Describes how will a resource be used
	enum class FGResourceAccessOp
	{
		Read,
		Write,
		WriteRead,
	};

	// Contains information about a resource that will be used in a RenderPass
	struct FGPassResourceUsageInfo
	{
		FGResourceID       m_ID;
		FGResourceType     m_Type;
		FGResourceAccessOp m_AccessOp;
		FGResourceSource   m_Source;
	};

	// Contains access flags for a texture resource used to create a barrier
	struct FGPipelineAccessInfo
	{
		PipelineStage     m_Stage = PipelineStage::TopOfPipe;
		AccessMask        m_Access = AccessMask::None;
		TextureLayout     m_Layout = TextureLayout::Undefined;
		TextureAspectMask m_Aspect = TextureAspectMask::Color;
		LoadOp            m_LoadOp = LoadOp::DontCare;

		bool operator==(FGPipelineAccessInfo const& rhs) const;

		static FGPipelineAccessInfo FragmentShaderRead();
		static FGPipelineAccessInfo ColorAttachmentWrite();
		static FGPipelineAccessInfo DepthStencil();
	};

	struct FGTextureAccessInfo
	{
		FGResourceID         m_ID;
		FGPipelineAccessInfo m_Access;
	};
}

namespace CKE {
	struct FGResourceInfo
	{
		FGResourceType   m_Type;
		FGResourceSource m_Source;
	};

	struct FGBufferData
	{
		FGResourceID m_ID;
		BufferHandle m_Handle;
	};

	struct FGTransientBuffer
	{
		BufferDesc m_Desc;
	};

	struct FGImportedBuffer { };

	// Base info that all the textures have
	struct FGTextureData
	{
		FGResourceID         m_ID;
		TextureHandle        m_TexHandle;
		TextureViewHandle    m_ViewHandle;    // View of the complete texture
		TextureDesc          m_TexDesc;       // Description used to create the texture
		FGPipelineAccessInfo m_InitialAccess; // Access state at the start of the frame
	};

	struct TextureExtraSettings
	{
		bool m_UseSizeRelativeToRenderTarget = false;
		Vec2 m_RelativeSize{1.0f, 1.0f};
	};

	struct FGTransientTexture
	{
		TextureExtraSettings m_Extra;
	};

	struct FGImportedTexture { };
}

namespace CKE {
	enum class RenderPassType
	{
		Graphics,
		Transfer,
		Compute
	};

	struct FGImportedTextureDesc
	{
		FGResourceID      m_fgID;
		TextureHandle     m_TexHandle;
		TextureDesc       m_TexDesc;
		TextureViewHandle m_FullTexView; // View of the complete texture

		// This info is used as Src for the first transition barrier
		TextureLayout m_InitialLayout;
		PipelineStage m_SrcStageWhenAvailable;
		AccessMask    m_SrcAccessMaskWhenAvailable;
		LoadOp        m_LoadOp;
	};
}
