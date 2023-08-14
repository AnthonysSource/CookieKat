#pragma once

#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

namespace CKE {
	class RenderDevice;
	class TextureSamplersCache;
	class ResourceSystem;

	class PresentPass : public FGGraphicsRenderPass
	{
	public:
		PresentPass() : FGGraphicsRenderPass{ "PresentPass" }{}

		void Initialize(RenderDevice* pDevice, TextureSamplersCache* pSamplerCache, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle m_Pipeline{};
		RenderDevice*  m_pDevice{nullptr};
		TextureSamplersCache* m_pSamplerCache = nullptr;
	};
}
