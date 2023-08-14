#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

namespace CKE {
	class FrameGraphSetupContext;
}

namespace CKE {
	class IntensityHistoryCopyPass : public FGTransferRenderPass
	{
	public:
		IntensityHistoryCopyPass() = default;
		IntensityHistoryCopyPass(FGRenderPassID renderPassID);

		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, TransferCommandList& cmdList, RenderDevice& rd) override;
	};

	//-----------------------------------------------------------------------------

	class IntensityCheckPass : public FGGraphicsRenderPass
	{
	public:
		inline static const String Intensity{"IntensityCheck"};
		inline static const String HistoryColor{"HistoryColor"};
		inline static const String HistoryObjectIdx{"HistoryObjectIdx"};

		IntensityCheckPass() = default;
		IntensityCheckPass(RenderPassInitCtx& initCtx);

		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		EntityDatabase*       m_pAdmin = nullptr;
		PipelineHandle        m_Pipeline;
		TextureSamplersCache* m_pSamplersCache = nullptr;
	};
}
