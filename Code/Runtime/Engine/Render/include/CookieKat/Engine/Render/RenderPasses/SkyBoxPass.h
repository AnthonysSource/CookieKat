#pragma once

#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"
#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

namespace CKE {
	class FrameGraphSetupContext;
}

namespace CKE {
	class SkyBoxPass : public FGGraphicsRenderPass
	{
	public:
		SkyBoxPass() : FGGraphicsRenderPass{"SkyBox Pass"} {}

		void Initialize(RenderPassInitCtx* pRenderCtx, ResourceSystem* pResources,
		                TextureViewHandle          skyboxView);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_Pipeline;
		TextureSamplersCache* m_pSamplerCache = nullptr;
		TextureViewHandle     m_SkyBoxViewHandle;
		RenderingSettings const*                   m_pView = nullptr;
	};
}
