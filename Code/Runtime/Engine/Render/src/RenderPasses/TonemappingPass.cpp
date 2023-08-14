#include "CookieKat/Engine/Render/RenderPasses/TonemappingPass.h"

#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"

namespace CKE {
	void ToneMappingPass::Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources) {
		m_pSamplerCache = pCtx->GetSamplerCache();
		m_pRenderingSettings = pCtx->GetRenderingSettings();

		// Pipeline
		Path pipelinePath = "Shaders/tonemapping.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			{
				TextureFormat::R8G8B8A8_SRGB,
			},
			TextureFormat::D24_UNORM_S8_UINT,
		};
		pipelineDesc.m_AttachmentsInfo = attachments;
		pipelineDesc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		pipelineDesc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::Never, false
		};

		m_Pipeline = pCtx->GetDevice()->CreateGraphicsPipeline(pipelineDesc);
	}

	void ToneMappingPass::Setup(FrameGraphSetupContext& setup) {
		TextureDesc toneMappedTexDesc{};
		toneMappedTexDesc.m_Name = "Tonemapped Output";
		toneMappedTexDesc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		toneMappedTexDesc.m_AspectMask = TextureAspectMask::Color;
		toneMappedTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(ToneMappped, toneMappedTexDesc
		                             , TextureExtraSettings{true, {1.0f, 1.0f}});
		setup.UseTexture(ToneMappped, FGPipelineAccessInfo::ColorAttachmentWrite());
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::FragmentShaderRead());
	}

	void ToneMappingPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                              RenderDevice&         rd) {
		TextureViewHandle sceneColor = ctx.GetTextureView(LightingPass::HDRSceneColor);
		TextureViewHandle tonemapped = ctx.GetTextureView(ToneMappped);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pRenderingSettings->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					tonemapped, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);

		// Set Pipeline Dynamic State
		cmdList.SetDefaultViewportScissor(m_pRenderingSettings->m_Viewport.m_Extent);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 materialDescriptor =
				b.BindTextureWithSampler(0, sceneColor, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, materialDescriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}
