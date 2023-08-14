#pragma once

namespace CKE {
	class DepthPrePass;
	class GBufferPass;
	class SSAOPass;
	class BlurPass;
	class LightingPass;
	class BloomModule;
	class SkyBoxPass;
	class ToneMappingPass;
	class FXAAPass;
	class PresentPass;

	class FrameGraph;
	class RenderPassInitCtx;
}

namespace CKE {
	class RenderPassPipeline
	{
	public:
		void Initialize(FrameGraph& graph, RenderPassInitCtx& ctx);
		void Shutdown();

	private:
		DepthPrePass* m_DepthPass{};
		GBufferPass* m_GBufferPass{};
		SSAOPass* m_SSAOPass{};
		BlurPass* m_SSAOBlurPass{};
		LightingPass* m_LightingPass{};
		BloomModule* m_BloomModule{};
		SkyBoxPass* m_SkyBoxPass{};
		ToneMappingPass* m_TonemappingPass{};
		FXAAPass* m_FXAAPass{};
		PresentPass* m_PresentPass{};
	};
}
