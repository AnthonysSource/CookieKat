#pragma once

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

#include "CookieKat/Engine/Entities/EntitySystem.h"

namespace CKE {
	class GBufferPass : public FGGraphicsRenderPass
	{
	public:
		GBufferPass() : FGGraphicsRenderPass{"GBufferPass"} {}

		void Initialize(RenderPassInitCtx* pCtx);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, CommandList& cmdList, RenderDevice& rd) override;

	private:
		RenderDevice*            m_pDevice = nullptr;
		TextureSamplersCache*    m_pSamplerCache = nullptr;
		EntityDatabase*          m_pEntityDB = nullptr;
		ResourceSystem*          m_pResources = nullptr;
		RenderingSettings const* m_pRenderingSettings = nullptr;

		PipelineHandle m_Pipeline;
	};
}
