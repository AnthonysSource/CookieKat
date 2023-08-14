#include "CookieKat/Engine/Render/RenderPasses/SSAOPass.h"

#include "CookieKat/Core/Random/Random.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"
#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Engine/Entities/Components/CameraComponent.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"

namespace CKE {
	void SSAOPass::Initialize(RenderPassInitCtx* pCtx, EntityDatabase* pAdmin,
	                          ResourceSystem*    pResources) {
		m_pEntityDB = pCtx->GetEntityDatabase();
		m_pSamplerCache = pCtx->GetSamplerCache();
		m_pRenderingSettings = pCtx->GetRenderingSettings();

		Path pipelinePath = "Shaders/ssao.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R8G8B8A8_UNORM,
			}
		};
		pipelineDesc.m_AttachmentsInfo = attachments;
		pipelineDesc.m_BlendState = BlendState{
			{AttachmentBlendState{}}
		};
		pipelineDesc.m_DepthStencilState = DepthStencilState{
			false, false, CompareOp::Never, false
		};

		m_Pipeline = pCtx->GetDevice()->CreateGraphicsPipeline(pipelineDesc);
		pResources->UnloadResource(pipelineResourceID);

		// Generation of SSAO samples and Semi-sphere rotations
		//-----------------------------------------------------------------------------

		constexpr i32 kernelSize = 64;
		for (int i = 0; i < kernelSize; ++i) {
			// Generate random positions inside the hemisphere
			Vec4 sample = Vec4{
				Random::F32(0, 1) * 2.0f - 1.0f,
				Random::F32(0, 1) * 2.0f - 1.0f,
				Random::F32(0, 1),
				0.0f
			};
			sample = glm::normalize(sample);
			sample *= Random::F32(0, 1);

			// We do this scaling because we want most samples to be closer
			// to the fragment original position because those should have
			// more weight in the final result
			f32 scale = (f32)i / kernelSize;
			scale = std::lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			m_SamplingData.m_Kernel[i] = sample;
		}

		constexpr i32 noiseSize = 16;
		for (int i = 0; i < noiseSize; ++i) {
			Vec4 noise = Vec4{
				Random::F32(0, 1) * 2.0f - 1.0f,
				Random::F32(0, 1) * 2.0f - 1.0f,
				0.0f,
				0.0f
			};
			m_SamplingData.m_Noise[i] = noise;
		}
	}

	void SSAOPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(GBuffer::Position, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::Normals, FGPipelineAccessInfo::FragmentShaderRead());
		setup.UseTexture(GBuffer::DepthStencil, FGPipelineAccessInfo{
			                 .m_Stage = PipelineStage::FragmentShader,
			                 .m_Access = AccessMask::Shader_Read,
			                 .m_Layout = TextureLayout::Shader_ReadOnly,
			                 .m_Aspect = TextureAspectMask::Depth,
			                 .m_LoadOp = LoadOp::Load
		                 });
		setup.UseBuffer(SceneGlobal::View);

		TextureDesc ssaoTexDesc{};
		ssaoTexDesc.m_Name = "SSAO Texture";
		ssaoTexDesc.m_Format = TextureFormat::R8G8B8A8_UNORM;
		ssaoTexDesc.m_AspectMask = TextureAspectMask::Color;
		ssaoTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(SSAO_Raw, ssaoTexDesc,
		                             TextureExtraSettings{true, Vec2{1.0f, 1.0f}});
		setup.UseTexture(SSAO_Raw, FGPipelineAccessInfo::ColorAttachmentWrite());

		BufferDesc samplingBufferDesc{};
		samplingBufferDesc.m_Name = "Sampling Data";
		samplingBufferDesc.m_Usage = BufferUsage::Uniform | BufferUsage::TransferDst;
		samplingBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		samplingBufferDesc.m_UpdateFrequency = UpdateFrequency::Static;
		samplingBufferDesc.m_SizeInBytes = sizeof(SSAOSamplingDataGPU);
		setup.CreateTransientBuffer(SamplingBuffer, samplingBufferDesc);
		setup.UseBuffer(SamplingBuffer);
	}

	void SSAOPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                       RenderDevice&         rd) {
		BufferHandle sampling = ctx.GetBuffer(SamplingBuffer);
		BufferHandle view = ctx.GetBuffer(SceneGlobal::View);

		TextureViewHandle positionTex = ctx.GetTextureView(GBuffer::Position);
		TextureViewHandle normalTex = ctx.GetTextureView(GBuffer::Normals);
		TextureViewHandle depthTex = ctx.GetTextureView(GBuffer::DepthStencil);

		TextureViewHandle ssaoOutputTex = ctx.GetTextureView(SSAO_Raw);

		rd.UploadBufferData_DEPR(sampling, &m_SamplingData, sizeof(SSAOSamplingDataGPU), 0);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pRenderingSettings->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					ssaoOutputTex,
					TextureLayout::Color_Attachment,
					LoadOp::DontCare, StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pRenderingSettings->m_Viewport.m_Extent);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder builder = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		DescriptorSetHandle  setHandle =
				builder.BindTextureWithSampler(0, positionTex, sampler)
				       .BindTextureWithSampler(1, normalTex, sampler)
				       .BindTextureWithSampler(2, depthTex, sampler)
				       .BindUniformBuffer(3, view)
				       .BindUniformBuffer(4, sampling)
				       .Build();
		cmdList.BindDescriptor(m_Pipeline, setHandle);

		// Screen Quad
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}

namespace CKE {
	void BlurPass::Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources) {
		m_pSamplerCache = pCtx->GetSamplerCache();
		m_pRenderingSettings = pCtx->GetRenderingSettings();

		Path pipelinePath = "Shaders/blur.pipeline";
		auto pipelineResourceID = pResources->LoadResource<PipelineResource>(pipelinePath);
		auto pipelineResource = pResources->GetResource<PipelineResource>(pipelineResourceID);

		GraphicsPipelineDesc pipelineDesc;
		pipelineDesc.m_VertexShaderSource = pipelineResource->GetVertSource();
		pipelineDesc.m_FragmentShaderSource = pipelineResource->GetFragSource();
		pipelineDesc.m_LayoutHandle = pipelineResource->GetPipelineLayout();
		pipelineDesc.m_VertexInput = pipelineResource->GetVertexInputLayoutDesc();

		AttachmentsInfo attachments{
			.m_ColorAttachments = {
				TextureFormat::R8G8B8A8_UNORM,
			}
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

	void BlurPass::Setup(FrameGraphSetupContext& setup) {
		TextureDesc ssaoBlurredTexDesc{};
		ssaoBlurredTexDesc.m_Name = "SSAO Blurred Texture";
		ssaoBlurredTexDesc.m_Format = TextureFormat::R8G8B8A8_UNORM;
		ssaoBlurredTexDesc.m_AspectMask = TextureAspectMask::Color;
		ssaoBlurredTexDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(SSAOPass::SSAO_Blurred, ssaoBlurredTexDesc,
		                             TextureExtraSettings{true, Vec2{1.0f, 1.0f}});
		setup.UseTexture(SSAOPass::SSAO_Blurred, FGPipelineAccessInfo::ColorAttachmentWrite());
		setup.UseTexture(SSAOPass::SSAO_Raw, FGPipelineAccessInfo::FragmentShaderRead());
	}

	void BlurPass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList,
	                       RenderDevice&         rd) {
		TextureViewHandle ssaoRaw = ctx.GetTextureView(SSAOPass::SSAO_Raw);
		TextureViewHandle blurredOut = ctx.GetTextureView(SSAOPass::SSAO_Blurred);

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pRenderingSettings->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					.m_TextureView = blurredOut,
					.m_Layout = TextureLayout::Color_Attachment,
					.m_LoadOp = LoadOp::DontCare,
					.m_StoreOp = StoreOp::Store,
				}
			},
			.m_UseDepthAttachment = false,
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pRenderingSettings->m_Viewport.m_Extent);

		// Shader Bindings
		SamplerHandle        sampler = m_pSamplerCache->CreateSampler(SamplerDesc{});
		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		auto                 bindings =
				b.BindTextureWithSampler(0, ssaoRaw, sampler)
				 .Build();
		cmdList.BindDescriptor(m_Pipeline, bindings);

		// Buffers
		cmdList.SetVertexBuffer(GlobalRenderAssets::GetScreenQuad());
		cmdList.Draw(6, 1, 0, 0);

		cmdList.EndRendering();
	}
}
