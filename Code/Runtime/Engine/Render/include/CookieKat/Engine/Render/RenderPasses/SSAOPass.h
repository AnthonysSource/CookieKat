#pragma once

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

namespace CKE {
	class FrameGraphSetupContext;
}

namespace CKE {
	// SSAO Data uploaded to the GPU
	struct SSAOSamplingDataGPU
	{
		Vec4 m_Kernel[64];
		Vec4 m_Noise[16];
	};

	class SSAOPass : public FGGraphicsRenderPass
	{
	public:
		inline static const String SamplingBuffer = "SamplingData";
		inline static const String SSAO_Raw = "SSAO_Raw";
		inline static const String SSAO_Blurred = "SSAO_Blurred";

	public:
		SSAOPass() : FGGraphicsRenderPass{FGRenderPassID{"SSAOPass"}} {}

		void Initialize(RenderPassInitCtx* pCtx, EntityDatabase* pAdmin,
		                ResourceSystem*    pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle           m_Pipeline{};
		SSAOSamplingDataGPU      m_SamplingData{};
		EntityDatabase*          m_pEntityDB{nullptr};
		TextureSamplersCache*    m_pSamplerCache = nullptr;
		RenderingSettings const* m_pRenderingSettings = nullptr;
	};

	class BlurPass : public FGGraphicsRenderPass
	{
	public:
		BlurPass() : FGGraphicsRenderPass{FGRenderPassID{"SSAO Blur"}} {}

		void Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle           m_Pipeline{};
		TextureSamplersCache*    m_pSamplerCache{};
		RenderingSettings const* m_pRenderingSettings = nullptr;
	};
}
