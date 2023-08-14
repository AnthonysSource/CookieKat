#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"

#include "CookieKat/Engine/Entities/Components/CameraComponent.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Render/RenderPasses/SSAOPass.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"

namespace CKE {
	void LightingPass::Initialize(RenderPassInitCtx* pCtx) {
		m_pDevice = pCtx->GetDevice();
		m_pSamplersCache = pCtx->GetSamplerCache();
		m_pView = pCtx->GetRenderingSettings();

		// Load Pipeline Resource
		Path pipelinePath = "Shaders/lighting.pipeline";
		auto pipelineResourceID = pCtx->GetResourceSystem()->LoadResource<PipelineResource>(
			pipelinePath);
		auto pipelineResource = pCtx->GetResourceSystem()->GetResource<PipelineResource>(
			pipelineResourceID);

		// Generate pipeline desc from resource data
		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		// Attachments
		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R32G32B32A32_SFLOAT,
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

	void LightingPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(GBuffer::Albedo, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::Normals, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::Position, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::RoughMetalRefl, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(SSAOPass::SSAO_Blurred, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseBuffer(SceneGlobal::View);
		setup.UseBuffer(SceneGlobal::Lights);
		setup.UseBuffer(SceneGlobal::EnviorementData);

		TextureDesc sceneColorDesc{};
		sceneColorDesc.m_Name = "Scene Color";
		sceneColorDesc.m_Format = TextureFormat::R32G32B32A32_SFLOAT;
		sceneColorDesc.m_AspectMask = TextureAspectMask::Color;
		sceneColorDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(LightingPass::HDRSceneColor, sceneColorDesc,
		                             TextureExtraSettings{true, {1.0f, 1.0f}});
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void LightingPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                           RenderDevice&         rd) {
		TextureViewHandle sceneColor = ctx.GetTextureView(LightingPass::HDRSceneColor);

		TextureViewHandle albedoTex = ctx.GetTextureView(GBuffer::Albedo);
		TextureViewHandle normalsTex = ctx.GetTextureView(GBuffer::Normals);
		TextureViewHandle positionTex = ctx.GetTextureView(GBuffer::Position);
		TextureViewHandle roughnessMetallicTex = ctx.GetTextureView(GBuffer::RoughMetalRefl);
		TextureViewHandle ssaoTex = ctx.GetTextureView(SSAOPass::SSAO_Blurred);

		BufferHandle lightsBuffer = ctx.GetBuffer(SceneGlobal::Lights);
		BufferHandle viewBuffer = ctx.GetBuffer(SceneGlobal::View);
		BufferHandle envBuffer = ctx.GetBuffer(SceneGlobal::EnviorementData);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pView->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					sceneColor, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pView->m_Viewport.m_Extent);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplersCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 materialDescriptor =
				b.BindUniformBuffer(0, lightsBuffer)
				 .BindTextureWithSampler(1, albedoTex, sampler)
				 .BindTextureWithSampler(2, normalsTex, sampler)
				 .BindTextureWithSampler(3, roughnessMetallicTex, sampler)
				 .BindTextureWithSampler(4, positionTex, sampler)
				 .BindTextureWithSampler(5, ssaoTex, sampler)
				 .BindUniformBuffer(6, envBuffer)
				 .BindUniformBuffer(7, viewBuffer)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, materialDescriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}
