#pragma once
#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

namespace CKE {
	class LightingPass;
	class BlurPass;
	class SSAOPass;
	class GBufferPass;
	class DepthPrePass;
	class SkyBoxPass;
	class RenderSceneManager;
	class GraphicsCommandList;
}

namespace CKE {
	struct SHCoeffs9
	{
		Array<Vec3, 9> m_Value;
	};

	class LightProbeData
	{
	public:
		Int3      m_LocalPos{};
		SHCoeffs9 m_Coeffs{};
	};

	class LightProbeGrid
	{
	public:
		void      Setup(Mat4 gridToWorld);
		SHCoeffs9 GetInterpolatedSHCoeffsAtPosition(Vec3 worldPos);

	private:
		void            SetProbe(Int3 localPos, SHCoeffs9 coeffs);
		LightProbeData* GetProbe(Int3 localPos);

	private:
		Mat4                                   m_WorldToLightGrid = glm::identity<glm::mat4>();
		Vector<Vector<Vector<LightProbeData>>> m_Probes{};
		f32                                    m_SeparationBetweenProbes = 0.0f;
	};
}

namespace CKE {
	class CopyToCubeMapPass : public FGTransferRenderPass
	{
	public:
		CopyToCubeMapPass() : FGTransferRenderPass{"CubeMap Generation Pass"} {}

		void SetFaceToCopy(i32 faceIdx);
		void SetTargetCubeMap(TextureHandle cubeMap);
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, TransferCommandList& cmdList, RenderDevice& rd) override;

	private:
		i32           m_CurrentFaceIdx = 0;
		TextureHandle m_TargetCubeMap;
	};
}

namespace CKE {
	class LightProbeBakingSystem
	{
	public:
		void Initialize(RenderDevice*       pDevice,
		                RenderSceneManager* pSceneData,
		                DepthPrePass*       DepthPass,
		                GBufferPass*        GBufferPass,
		                SSAOPass*           SSAOPass,
		                BlurPass*           SSAOBlurPass,
		                LightingPass*       LightingPass,
		                SkyBoxPass*         skyboxPass);
		void           Shutdown();
		SHCoeffs9      RenderProbe(Vec3 pos);
		LightProbeGrid GenerateProbeGrid();

	private:
		RenderDevice*       m_pDevice{nullptr};
		RenderSceneManager* m_pRenderScene{nullptr};
		FrameGraph          m_FrameGraph{};
		CopyToCubeMapPass   m_CopyToCubemapPass{};
		BufferHandle        m_ReadBackBuffer{};
		TextureHandle       m_CubeMapTex{};
	};
}
