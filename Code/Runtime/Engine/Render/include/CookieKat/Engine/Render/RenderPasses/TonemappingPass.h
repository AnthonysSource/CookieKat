#pragma once

#include "RenderPassInitCtx.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

namespace CKE {
	class ToneMappingPass : public FGGraphicsRenderPass
	{
	public:
		inline static const String ToneMappped{"TonemappingOutput"};

	public:
		ToneMappingPass() : FGGraphicsRenderPass{"ToneMappingPass"} {}

		void Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_Pipeline{};
		TextureSamplersCache* m_pSamplerCache = nullptr;
		RenderingSettings const* m_pRenderingSettings = nullptr;
	};
}
