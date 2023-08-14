#pragma once
#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	struct ParticleDataGPU
	{
		Vec4 m_Pos;
		Vec4 m_Velocity;
		Vec4 m_Color;
	};

	class ParticleSystem
	{
	public:
		void Initialize(RenderDevice* pDevice);
		void Update();
		void Shutdown();

	private:
		RenderDevice*               m_pDevice = nullptr;
		Array<ParticleDataGPU, 256> m_ParticlesInitialData{};
		BufferHandle                m_ParticlesCurrent{};
		BufferHandle                m_ParticlesLast{};
		PipelineHandle              m_ComputePipeline;
		PipelineHandle              m_GfxPipeline;
	};
}
