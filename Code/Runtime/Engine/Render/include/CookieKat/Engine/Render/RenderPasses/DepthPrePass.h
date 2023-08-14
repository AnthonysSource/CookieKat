#pragma once

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

namespace CKE {
	class FrameGraphSetupContext;
}

namespace CKE {
	class DepthPrePass : public FGGraphicsRenderPass
	{
	public:
		DepthPrePass() : FGGraphicsRenderPass{FGRenderPassID{"DepthPrePass"}} {}

		void Initialize(RenderPassInitCtx* pInitCtx);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		EntityDatabase*          m_pEntityDb = nullptr;
		ResourceSystem*          m_pResources = nullptr;
		RenderingSettings const* m_pRenderingSettings = nullptr;

		PipelineHandle m_Pipeline;
	};
}
