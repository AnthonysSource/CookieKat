#pragma once

#include "CookieKat/Systems/FrameGraph/FrameGraph.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

namespace CKE {
	class LightingPass : public FGGraphicsRenderPass
	{
	public:
		inline static const String HDRSceneColor{"LightingOutput"};

	public:
		LightingPass() : FGGraphicsRenderPass{ FGRenderPassID{"LightingPass"}}{}
		void Initialize(RenderPassInitCtx* pCtx);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		RenderDevice*            m_pDevice = nullptr;
		TextureSamplersCache*    m_pSamplersCache = nullptr;
		PipelineHandle           m_Pipeline{};
		RenderingSettings const* m_pView = nullptr;
	};
}
