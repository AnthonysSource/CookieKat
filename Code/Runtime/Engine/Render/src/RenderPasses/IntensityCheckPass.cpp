#pragma once

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/IntensityCheckPass.h"
#include "CookieKat/Engine/Render/RenderPasses/FXAAPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"

namespace CKE {
	IntensityHistoryCopyPass::IntensityHistoryCopyPass(FGRenderPassID renderPassID) : FGTransferRenderPass{
		renderPassID
	} { }

	void IntensityHistoryCopyPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo{
			                 .m_Stage = PipelineStage::Transfer,
			                 .m_Access = AccessMask::Transfer_Read,
			                 .m_Layout = TextureLayout::Transfer_Src,
			                 .m_Aspect = TextureAspectMask::Color,
			                 .m_LoadOp = LoadOp::Load
		                 });


		setup.UseTexture(IntensityCheckPass::HistoryColor, FGPipelineAccessInfo{
			                 .m_Stage = PipelineStage::Transfer,
			                 .m_Access = AccessMask::Transfer_Write,
			                 .m_Layout = TextureLayout::Transfer_Dst,
			                 .m_Aspect = TextureAspectMask::Color,
			                 .m_LoadOp = LoadOp::DontCare
		                 });
	}

	void IntensityHistoryCopyPass::Execute(ExecuteResourcesCtx& ctx, TransferCommandList& cmdList, RenderDevice& rd) {
		TextureHandle sceneColor = ctx.GetTexture(LightingPass::HDRSceneColor);
		TextureHandle historySceneColor = ctx.GetTexture(IntensityCheckPass::HistoryColor);

		TextureCopyInfo srcCopyInfo{};
		srcCopyInfo.m_Layout = TextureLayout::Transfer_Src;
		srcCopyInfo.m_TexHandle = sceneColor;
		TextureCopyInfo dstCopyInfo{};
		dstCopyInfo.m_Layout = TextureLayout::Transfer_Dst;
		dstCopyInfo.m_TexHandle = historySceneColor;
		cmdList.CopyTexture(srcCopyInfo, dstCopyInfo, rd.GetBackBufferSize3());
	}
}

namespace CKE {
	//-----------------------------------------------------------------------------

	IntensityCheckPass::IntensityCheckPass(RenderPassInitCtx& initCtx):
		FGGraphicsRenderPass{"IntensityCheckPass"} {
		m_pAdmin = initCtx.GetEntityDatabase();
		m_Pipeline = initCtx.GetPipelineManager()->GetPipeline(PipelineIDS::IntensityCheckPass);
		m_pSamplersCache = initCtx.GetSamplerCache();
	}

	void IntensityCheckPass::Setup(FrameGraphSetupContext& setup) {
		TextureDesc desc{};
		desc.m_Name = "Intensity Texture";
		desc.m_AspectMask = TextureAspectMask::Color;
		desc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		desc.m_Usage = TextureUsage::Color_Attachment | TextureUsage::Sampled;
		desc.m_TextureType = TextureType::Tex2D;
		TextureExtraSettings ext{};
		ext.m_RelativeSize = Vec2{1.0f, 1.0f};
		ext.m_UseSizeRelativeToRenderTarget = true;
		setup.CreateTransientTexture(IntensityCheckPass::Intensity, desc, ext);

		setup.UseTexture(IntensityCheckPass::Intensity, FGPipelineAccessInfo::ColorAttachmentWrite());
		setup.UseTexture(IntensityCheckPass::HistoryColor, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(IntensityCheckPass::HistoryObjectIdx, FGPipelineAccessInfo::FragmentShaderRead());

		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::ObjectIdx, FGPipelineAccessInfo::FragmentShaderRead());
	}

	void IntensityCheckPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) {
		TextureViewHandle intensityTex = ctx.GetTextureView(IntensityCheckPass::Intensity);
		TextureViewHandle lastSceneColor = ctx.GetTextureView(IntensityCheckPass::HistoryColor);
		TextureViewHandle lastObjIdxTex = ctx.GetTextureView(IntensityCheckPass::HistoryObjectIdx);

		TextureViewHandle sceneColor = ctx.GetTextureView(LightingPass::HDRSceneColor);
		TextureViewHandle objIdxTex = ctx.GetTextureView(GBuffer::ObjectIdx);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = rd.GetBackBufferSize(),
			.m_ColorAttachments = {
				RenderingAttachment{
					intensityTex, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(rd.GetBackBufferSize());

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplersCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder descriptorSetBuilder = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 descriptorSet =
				descriptorSetBuilder.BindTextureWithSampler(0, sceneColor, sampler)
				                    .BindTextureWithSampler(1, lastSceneColor, sampler)
				                    .BindTextureWithSampler(2, objIdxTex, sampler)
				                    .BindTextureWithSampler(3, lastObjIdxTex, sampler)
				                    .Build();
		cmdList.BindDescriptor(m_Pipeline, descriptorSet);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}
