#include "CookieKat/Engine/Render/RenderPasses/FXAAPass.h"

#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"

#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/TonemappingPass.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphBuilder.h"

namespace CKE {
	void FXAAPass::Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources) {
		m_ID = FGRenderPassID{"FXAAPass"};
		m_pDevice = pCtx->GetDevice();
		m_pSamplerCache = pCtx->GetSamplerCache();
		m_pView = pCtx->GetRenderingSettings();

		// Pipeline
		Path pipelinePath = "Shaders/FXAA.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R8G8B8A8_SRGB,
			},
			.m_DepthStencil = TextureFormat::D24_UNORM_S8_UINT,
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

	void FXAAPass::Setup(FrameGraphSetupContext& setup) {
		TextureDesc outputTexDesc{};
		outputTexDesc.m_Name = "FXAA Output";
		outputTexDesc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		outputTexDesc.m_AspectMask = TextureAspectMask::Color;
		outputTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(FXAAPass::FXAA_Output, outputTexDesc,
		                             TextureExtraSettings{true, {1.0f, 1.0f}});
		setup.UseTexture(FXAAPass::FXAA_Output, FGPipelineAccessInfo::ColorAttachmentWrite());
		setup.UseTexture(ToneMappingPass::ToneMappped, FGPipelineAccessInfo::FragmentShaderRead());
	}

	void FXAAPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                       RenderDevice&         rd) {
		TextureViewHandle sceneColor = ctx.GetTextureView(ToneMappingPass::ToneMappped);
		TextureViewHandle fxaaOutput = ctx.GetTextureView(FXAAPass::FXAA_Output);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pView->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					fxaaOutput, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);

		// Set Pipeline Dynamic State
		cmdList.SetDefaultViewportScissor(m_pView->m_Viewport.m_Extent);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 descriptor =
				b.BindTextureWithSampler(0, sceneColor, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, descriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}
