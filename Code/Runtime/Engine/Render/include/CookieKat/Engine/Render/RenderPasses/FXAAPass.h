#pragma once

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

namespace CKE {
	class FXAAPass : public FGGraphicsRenderPass
	{
	public:
		inline static const String FXAA_Output{"FXAA_Output"};

	public:
		FXAAPass() : FGGraphicsRenderPass{"FXAAPass"} {}
		void Initialize(RenderPassInitCtx* pCtx, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		RenderDevice*            m_pDevice = nullptr;
		TextureSamplersCache*    m_pSamplerCache = nullptr;
		PipelineHandle           m_Pipeline;
		RenderingSettings const* m_pView = nullptr;
	};
}
