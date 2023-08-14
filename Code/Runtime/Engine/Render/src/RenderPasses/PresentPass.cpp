#include "CookieKat/Engine/Render/RenderPasses/PresentPass.h"
#include "CookieKat/Engine/Render/RenderPasses/FXAAPass.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderPasses/IntensityCheckPass.h"

#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"

namespace CKE {
	void PresentPass::Initialize(RenderDevice*   pDevice, TextureSamplersCache* pSamplerCache,
	                             ResourceSystem* pResources) {
		m_ID = FGRenderPassID{"PresentPass"};
		m_pDevice = pDevice;
		m_pSamplerCache = pSamplerCache;

		// Pipeline
		Path pipelinePath = "Shaders/passThrough.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::B8G8R8A8_SRGB,
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

		m_Pipeline = pDevice->CreateGraphicsPipeline(pipelineDesc);
	}

	void PresentPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(FXAAPass::FXAA_Output, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(General::Swapchain, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void PresentPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) {
		TextureViewHandle finalColor = ctx.GetTextureView(FXAAPass::FXAA_Output);
		TextureViewHandle swapchain = ctx.GetTextureView(General::Swapchain);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = rd.GetBackBufferSize(),
			.m_ColorAttachments = {
				RenderingAttachment{
					swapchain, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);

		// Set Pipeline Dynamic State
		cmdList.SetDefaultViewportScissor(rd.GetBackBufferSize());

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 materialDescriptor =
				b.BindTextureWithSampler(0, finalColor, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, materialDescriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();

		cmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::ColorAttachmentOutput,
			.m_SrcAccessMask = AccessMask::ColorAttachment_Write,
			.m_DstStage = PipelineStage::BottomOfPipe,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Color_Attachment,
			.m_NewLayout = TextureLayout::Present_Src,
			.m_Texture = ctx.GetTexture(General::Swapchain),
			.m_AspectMask = TextureAspectMask::Color
		});
	}
}
