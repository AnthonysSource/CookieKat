#include "CookieKat/Engine/Render/RenderPasses/SkyBoxPass.h"

#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"
#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"

namespace CKE {
	void SkyBoxPass::Initialize(RenderPassInitCtx* pRenderCtx, ResourceSystem* pResources,
	                            TextureViewHandle  skyboxView) {
		m_pSamplerCache = pRenderCtx->GetSamplerCache();
		m_SkyBoxViewHandle = skyboxView;
		m_pView = pRenderCtx->GetRenderingSettings();

		// Pipeline
		Path pipelinePath = "Shaders/skybox.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc desc;
		desc.m_VertexShaderSource = pipelineResource->GetVertSource();
		desc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		desc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		desc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R32G32B32A32_SFLOAT,
			},
			.m_DepthStencil = TextureFormat::D24_UNORM_S8_UINT,
		};
		desc.m_AttachmentsInfo = attachments;
		desc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		desc.m_DepthStencilState = DepthStencilState{
			true, false, CompareOp::LessOrEqual, false
		};

		m_Pipeline = pRenderCtx->GetDevice()->CreateGraphicsPipeline(desc);
	}

	void SkyBoxPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseBuffer(SceneGlobal::View);
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::ColorAttachmentWrite());
		setup.UseTexture(GBuffer::DepthStencil, FGPipelineAccessInfo::DepthStencil());
	}

	void SkyBoxPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) {
		BufferHandle      view = ctx.GetBuffer(SceneGlobal::View);
		TextureViewHandle sceneColor = ctx.GetTextureView(LightingPass::HDRSceneColor);
		TextureViewHandle depthBuffer = ctx.GetTextureView(GBuffer::DepthStencil);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pView->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					sceneColor,
					TextureLayout::Color_Attachment,
					LoadOp::Load, StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = true,
			.m_DepthAttachment = RenderingAttachment{
				depthBuffer,
				TextureLayout::DepthStencil_Attachment,
				LoadOp::Load, StoreOp::Store,
			},
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pView->m_Viewport.m_Extent);

		DescriptorSetBuilder setBuilder = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		DescriptorSetHandle  setHandle = setBuilder
		                                 .BindUniformBuffer(0, view)
		                                 .BindTextureWithSampler(1, m_SkyBoxViewHandle,
		                                                         m_pSamplerCache->CreateSampler(SamplerDesc{}))
		                                 .Build();
		cmdList.BindDescriptor(m_Pipeline, setHandle);

		cmdList.SetVertexBuffer(GlobalRenderAssets::GetCubeMapMesh());
		cmdList.Draw(36, 1, 0, 0);

		cmdList.EndRendering();
	}
}
