#pragma once
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"

namespace CKE {
	class FrameGraphSetupContext;
}

namespace CKE {
	class BloomPreFilterPass : public FGGraphicsRenderPass
	{
	public:
		BloomPreFilterPass() : FGGraphicsRenderPass{"Bloom Module"} {}
		void Initialize(RenderPassInitCtx* pRenderSubSystems, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_PrePassPipeline{};
		TextureSamplersCache* m_pSamplerCache{};
	};
}

namespace CKE {
	class BloomDownSamplePass : public FGGraphicsRenderPass
	{
	public:
		void Initialize(FGRenderPassID id, RenderPassInitCtx* pRenderSubSystems, ResourceSystem* pResources,
		                i32            stage);
		void SetInputOutput(FGResourceID input, FGResourceID output);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_BloomDownPipeline{};
		i32                   m_Stage{}; // Used to calculate the target texture size from the base original size
		FGResourceID          m_Input;
		FGResourceID          m_Output;
		TextureSamplersCache* m_pSamplerCache{};
	};

	class BloomUpSamplePass : public FGGraphicsRenderPass
	{
	public:
		void Initialize(FGRenderPassID id, RenderPassInitCtx* pRenderSubSystems, ResourceSystem* pResources,
		                i32            stage);
		void SetInputOutput(FGResourceID input, FGResourceID combineSrc, FGResourceID output);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_Pipeline{};
		i32                   m_Stage{}; // Used to calculate the target texture size from the base original size
		FGResourceID          m_Input;
		FGResourceID          m_CombineSrc;
		FGResourceID          m_Output;
		TextureSamplersCache* m_pSamplerCache{};
	};
}

namespace CKE {
	class BloomCombinePass : public FGGraphicsRenderPass
	{
	public:
		BloomCombinePass() : FGGraphicsRenderPass{"Bloom Combine"} {}
		void Initialize(RenderPassInitCtx* pRenderSubSystems, ResourceSystem* pResources);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override;

	private:
		PipelineHandle        m_Pipeline{};
		TextureSamplersCache* m_pSamplerCache{};
	};
}

namespace CKE {
	// Contains all of the necessary passes to calculate bloom contribution to the scene color
	class BloomModule
	{
	public:
		inline static const String BloomPreFilter = "BloomPreFilter";

		inline static const String BloomDown_1 = "BloomDowns_1";
		inline static const String BloomDown_2 = "BloomDowns_2";
		inline static const String BloomDown_3 = "BloomDowns_3";
		inline static const String BloomDown_4 = "BloomDowns_4";

		inline static const String BloomDown_5 = "BloomDowns_5";

		inline static const String BloomUp_4 = "BloomUp_4";
		inline static const String BloomUp_3 = "BloomUp_3";
		inline static const String BloomUp_2 = "BloomUp_2";
		inline static const String BloomUp_1 = "BloomUp_1";
		inline static const String BloomUp_0 = "BloomUp_0";

	public:
		void Initialize(RenderPassInitCtx* initCtx, ResourceSystem* pResources);
		void AddToGraph(FrameGraph* frameGraph);

	private:
		BloomPreFilterPass m_BloomPreFilter{};

		BloomDownSamplePass m_BloomDownSample_1{};
		BloomDownSamplePass m_BloomDownSample_2{};
		BloomDownSamplePass m_BloomDownSample_3{};
		BloomDownSamplePass m_BloomDownSample_4{};
		BloomDownSamplePass m_BloomDownSample_5{};

		BloomUpSamplePass m_BloomUpSample_4{};
		BloomUpSamplePass m_BloomUpSample_3{};
		BloomUpSamplePass m_BloomUpSample_2{};
		BloomUpSamplePass m_BloomUpSample_1{};
		BloomUpSamplePass m_BloomUpSample_0{};

		BloomCombinePass m_BloomCombine{};
	};
}
