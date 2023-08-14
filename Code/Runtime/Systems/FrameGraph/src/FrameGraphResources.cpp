#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"

#include "CookieKat/Systems/RenderAPI/Pipeline.h"

namespace CKE {
	bool FGPipelineAccessInfo::operator==(FGPipelineAccessInfo const& rhs) const {
		return
				m_Stage == rhs.m_Stage &&
				m_Access == rhs.m_Access &&
				m_Layout == rhs.m_Layout &&
				m_Aspect == rhs.m_Aspect &&
				m_LoadOp == rhs.m_LoadOp;
	}

	FGPipelineAccessInfo FGPipelineAccessInfo::FragmentShaderRead() {
		FGPipelineAccessInfo shaderRead{
			.m_Stage = PipelineStage::FragmentShader,
			.m_Access = AccessMask::Shader_Read,
			.m_Layout = TextureLayout::Shader_ReadOnly,
			.m_Aspect = TextureAspectMask::Color,
			.m_LoadOp = LoadOp::Load
		};
		return shaderRead;
	}

	FGPipelineAccessInfo FGPipelineAccessInfo::ColorAttachmentWrite() {
		FGPipelineAccessInfo gBufferAccess{
			.m_Stage = PipelineStage::ColorAttachmentOutput,
			.m_Access = AccessMask::ColorAttachment_Write,
			.m_Layout = TextureLayout::Color_Attachment,
			.m_Aspect = TextureAspectMask::Color,
			.m_LoadOp = LoadOp::Clear
		};
		return gBufferAccess;
	}

	FGPipelineAccessInfo FGPipelineAccessInfo::DepthStencil() {
		return FGPipelineAccessInfo{
			.m_Stage = PipelineStage::EarlyFragmentTest | PipelineStage::LateFragmentTest,
			.m_Access = AccessMask::DepthStencilAttachment_Write |
			AccessMask::DepthStencilAttachment_Read,
			.m_Layout = TextureLayout::DepthStencil_Attachment,
			.m_Aspect = TextureAspectMask::Depth,
			.m_LoadOp = LoadOp::Clear
		};
	}
}
