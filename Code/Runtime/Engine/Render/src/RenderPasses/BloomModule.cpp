#include "RenderPasses/BloomModule.h"

#include "Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphBuilder.h"
#include "RenderPasses/LightingPass.h"

namespace CKE {
	void BloomPreFilterPass::Initialize(RenderPassInitCtx* pRenderCtx, ResourceSystem* pResources) {
		m_pSamplerCache = pRenderCtx->GetSamplerCache();

		// Pipeline
		Path pipelinePath = "Shaders/Post/bloomThreshold.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc desc;
		desc.m_VertexShaderSource = pipelineResource->GetVertSource();
		desc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		desc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		desc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		desc.m_AttachmentsInfo = AttachmentsInfo{
			.m_ColorAttachments = {
				TextureFormat::R16G16B16A16_SFLOAT,
			},
		};
		desc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		desc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::LessOrEqual, false
		};

		m_PrePassPipeline = pRenderCtx->GetDevice()->CreateGraphicsPipeline(desc);
	}

	void BloomPreFilterPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::FragmentShaderRead());

		TextureDesc bloomTexDesc{};
		bloomTexDesc.m_Name = "Bloom PreFilter";
		bloomTexDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		bloomTexDesc.m_AspectMask = TextureAspectMask::Color;
		bloomTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(BloomModule::BloomPreFilter, bloomTexDesc,
		                             TextureExtraSettings{true, Vec2{1.0f, 1.0f}});
		setup.UseTexture(BloomModule::BloomPreFilter, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void BloomPreFilterPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                                 RenderDevice&         rd) {
		TextureViewHandle sceneColorTex = ctx.GetTextureView(LightingPass::HDRSceneColor);
		TextureViewHandle bloomPreFilterTex = ctx.GetTextureView(BloomModule::BloomPreFilter);

		// Pre Filter
		//-----------------------------------------------------------------------------

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = rd.GetBackBufferSize(),
			.m_ColorAttachments = {
				RenderingAttachment{
					.m_TextureView = bloomPreFilterTex,
					.m_Layout = TextureLayout::Color_Attachment,
					.m_LoadOp = LoadOp::Clear,
					.m_StoreOp = StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_PrePassPipeline);

		// Set Pipeline Dynamic State
		cmdList.SetDefaultViewportScissor(rd.GetBackBufferSize());

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_PrePassPipeline, 0);
		auto                 descriptor =
				b.BindTextureWithSampler(0, sceneColorTex, sampler)
				 .Build();
		cmdList.BindDescriptor(m_PrePassPipeline, descriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}

	void BloomDownSamplePass::Initialize(FGRenderPassID  id, RenderPassInitCtx* pRenderCtx,
	                                     ResourceSystem* pResources,
	                                     i32             stage) {
		m_ID = id;
		m_Stage = stage;
		m_pSamplerCache = pRenderCtx->GetSamplerCache();

		// Pipeline
		Path pipelinePath = "Shaders/Post/bloomDownSample.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc desc;
		desc.m_VertexShaderSource = pipelineResource->GetVertSource();
		desc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		desc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		desc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		desc.m_AttachmentsInfo = AttachmentsInfo{
			.m_ColorAttachments = {
				TextureFormat::R16G16B16A16_SFLOAT,
			}
		};
		desc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		desc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::LessOrEqual, false
		};

		m_BloomDownPipeline = pRenderCtx->GetDevice()->CreateGraphicsPipeline(desc);
	}

	void BloomDownSamplePass::SetInputOutput(FGResourceID input, FGResourceID output) {
		m_Input = input;
		m_Output = output;
	}

	void BloomDownSamplePass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(m_Input, FGPipelineAccessInfo::FragmentShaderRead());

		f32 relSize = std::pow(2.0f, m_Stage);

		TextureDesc outputTexDesc{};
		outputTexDesc.m_Name = "Bloom DownSample";
		outputTexDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		outputTexDesc.m_AspectMask = TextureAspectMask::Color;
		outputTexDesc.m_TextureType = TextureType::Tex2D;
		TextureExtraSettings extraSettings{
			true, Vec2{1.0f / relSize, 1.0f / relSize}
		};
		setup.CreateTransientTexture(m_Output, outputTexDesc, extraSettings);
		setup.UseTexture(m_Output, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void BloomDownSamplePass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                                  RenderDevice&         rd) {
		TextureViewHandle inputTex = ctx.GetTextureView(m_Input);
		TextureViewHandle outputTex = ctx.GetTextureView(m_Output);

		Vec2 currentSize = rd.GetBackBufferSize();
		f32  relSize = std::pow(2.0f, m_Stage);
		currentSize /= relSize;

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = currentSize,
			.m_ColorAttachments = {
				RenderingAttachment{
					.m_TextureView = outputTex,
					.m_Layout = TextureLayout::Color_Attachment,
					.m_LoadOp = LoadOp::Clear,
					.m_StoreOp = StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_BloomDownPipeline);
		cmdList.SetDefaultViewportScissor(currentSize);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler({});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_BloomDownPipeline, 0);
		auto                 descriptor =
				b.BindTextureWithSampler(0, inputTex, sampler)
				 .Build();
		cmdList.BindDescriptor(m_BloomDownPipeline, descriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}

	void BloomUpSamplePass::Initialize(FGRenderPassID  id, RenderPassInitCtx* pRenderSubSystems,
	                                   ResourceSystem* pResources,
	                                   i32             stage) {
		m_ID = id;
		m_Stage = stage;
		m_pSamplerCache = pRenderSubSystems->GetSamplerCache();

		Path pipelinePath = "Shaders/Post/bloomUpSample.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc desc;
		desc.m_VertexShaderSource = pipelineResource->GetVertSource();
		desc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		desc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		desc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R16G16B16A16_SFLOAT,
			},
		};
		desc.m_AttachmentsInfo = attachments;
		desc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		desc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::LessOrEqual, false
		};

		m_Pipeline = pRenderSubSystems->GetDevice()->CreateGraphicsPipeline(desc);
		pResources->UnloadResource(pipelineResourceID);
	}

	void BloomUpSamplePass::SetInputOutput(FGResourceID input, FGResourceID combineSrc,
	                                       FGResourceID output) {
		m_Input = input;
		m_CombineSrc = combineSrc;
		m_Output = output;
	}

	void BloomUpSamplePass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(m_Input, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(m_CombineSrc, FGPipelineAccessInfo::FragmentShaderRead());

		f32 relSize = std::pow(2.0f, m_Stage);

		TextureDesc outputTexDesc{};
		outputTexDesc.m_Name = "Bloom UpSample";
		outputTexDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		outputTexDesc.m_AspectMask = TextureAspectMask::Color;
		outputTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(m_Output, outputTexDesc, TextureExtraSettings{
			                             true, Vec2{1.0f / relSize, 1.0f / relSize}
		                             });
		setup.UseTexture(m_Output, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void BloomUpSamplePass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                                RenderDevice&         rd) {
		TextureViewHandle input = ctx.GetTextureView(m_Input);
		TextureViewHandle output = ctx.GetTextureView(m_Output);
		TextureViewHandle combine = ctx.GetTextureView(m_CombineSrc);

		Vec2 currentSize = rd.GetBackBufferSize();
		currentSize /= std::pow(2.0f, m_Stage);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = currentSize,
			.m_ColorAttachments = {
				RenderingAttachment{
					.m_TextureView = output,
					.m_Layout = TextureLayout::Color_Attachment,
					.m_LoadOp = LoadOp::Clear,
					.m_StoreOp = StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(currentSize);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler({});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 bindings =
				b.BindTextureWithSampler(0, input, sampler)
				 .BindTextureWithSampler(1, combine, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, bindings);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}

namespace CKE {
	void BloomCombinePass::Initialize(RenderPassInitCtx* pRenderSubSystems,
	                                  ResourceSystem*    pResources) {
		m_pSamplerCache = pRenderSubSystems->GetSamplerCache();

		Path pipelinePath = "Shaders/Post/bloomCombine.pipeline";
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
			}
		};
		desc.m_AttachmentsInfo = attachments;
		desc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		desc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::LessOrEqual, false
		};


		m_Pipeline = pRenderSubSystems->GetDevice()->CreateGraphicsPipeline(desc);
		pResources->UnloadResource(pipelineResourceID);
	}

	void BloomCombinePass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(BloomModule::BloomPreFilter, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(BloomModule::BloomUp_0, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void BloomCombinePass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                               RenderDevice&         rd) {
		TextureViewHandle base = ctx.GetTextureView(BloomModule::BloomPreFilter);
		TextureViewHandle bloom = ctx.GetTextureView(BloomModule::BloomUp_0);
		TextureViewHandle sceneColor = ctx.GetTextureView(LightingPass::HDRSceneColor);

		Vec2 currentSize = rd.GetBackBufferSize();

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = currentSize,
			.m_ColorAttachments = {
				RenderingAttachment{
					.m_TextureView = sceneColor,
					.m_Layout = TextureLayout::Color_Attachment,
					.m_LoadOp = LoadOp::Clear,
					.m_StoreOp = StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(currentSize);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler({});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 materialDescriptor =
				b.BindTextureWithSampler(0, base, sampler)
				 .BindTextureWithSampler(1, bloom, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, materialDescriptor);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}

namespace CKE {
	void BloomModule::Initialize(RenderPassInitCtx* initCtx, ResourceSystem* pResources) {
		m_BloomPreFilter.Initialize(initCtx, pResources);

		m_BloomDownSample_1.Initialize("Bloom DownSample 1", initCtx, pResources, 1);
		m_BloomDownSample_1.SetInputOutput(BloomModule::BloomPreFilter, BloomModule::BloomDown_1);
		m_BloomDownSample_2.Initialize("Bloom DownSample 2", initCtx, pResources, 2);
		m_BloomDownSample_2.SetInputOutput(BloomModule::BloomDown_1, BloomModule::BloomDown_2);
		m_BloomDownSample_3.Initialize("Bloom DownSample 3", initCtx, pResources, 3);
		m_BloomDownSample_3.SetInputOutput(BloomModule::BloomDown_2, BloomModule::BloomDown_3);
		m_BloomDownSample_4.Initialize("Bloom DownSample 4", initCtx, pResources, 4);
		m_BloomDownSample_4.SetInputOutput(BloomModule::BloomDown_3, BloomModule::BloomDown_4);
		m_BloomDownSample_5.Initialize("Bloom DownSample 5", initCtx, pResources, 5);
		m_BloomDownSample_5.SetInputOutput(BloomModule::BloomDown_4, BloomModule::BloomDown_5);

		m_BloomUpSample_4.Initialize("Bloom UpSample 4", initCtx, pResources, 4);
		m_BloomUpSample_4.SetInputOutput(BloomModule::BloomDown_5, BloomModule::BloomDown_4,
		                                 BloomModule::BloomUp_4);
		m_BloomUpSample_3.Initialize("Bloom UpSample 3", initCtx, pResources, 3);
		m_BloomUpSample_3.SetInputOutput(BloomModule::BloomUp_4, BloomModule::BloomDown_3,
		                                 BloomModule::BloomUp_3);
		m_BloomUpSample_2.Initialize("Bloom UpSample 2", initCtx, pResources, 2);
		m_BloomUpSample_2.SetInputOutput(BloomModule::BloomUp_3, BloomModule::BloomDown_2,
		                                 BloomModule::BloomUp_2);
		m_BloomUpSample_1.Initialize("Bloom UpSample 1", initCtx, pResources, 1);
		m_BloomUpSample_1.SetInputOutput(BloomModule::BloomUp_2, BloomModule::BloomDown_1,
		                                 BloomModule::BloomUp_1);
		m_BloomUpSample_0.Initialize("Bloom UpSample 0", initCtx, pResources, 0);
		m_BloomUpSample_0.SetInputOutput(BloomModule::BloomUp_1, BloomModule::BloomPreFilter,
		                                 BloomModule::BloomUp_0);

		m_BloomCombine.Initialize(initCtx, pResources);
	}

	void BloomModule::AddToGraph(FrameGraph* frameGraph) {
		frameGraph->AddGraphicsPass(&m_BloomPreFilter);
		frameGraph->AddGraphicsPass(&m_BloomDownSample_1);
		frameGraph->AddGraphicsPass(&m_BloomDownSample_2);
		frameGraph->AddGraphicsPass(&m_BloomDownSample_3);
		frameGraph->AddGraphicsPass(&m_BloomDownSample_4);
		frameGraph->AddGraphicsPass(&m_BloomDownSample_5);
		frameGraph->AddGraphicsPass(&m_BloomUpSample_4);
		frameGraph->AddGraphicsPass(&m_BloomUpSample_3);
		frameGraph->AddGraphicsPass(&m_BloomUpSample_2);
		frameGraph->AddGraphicsPass(&m_BloomUpSample_1);
		frameGraph->AddGraphicsPass(&m_BloomUpSample_0);
		frameGraph->AddGraphicsPass(&m_BloomCombine);
	}
}
