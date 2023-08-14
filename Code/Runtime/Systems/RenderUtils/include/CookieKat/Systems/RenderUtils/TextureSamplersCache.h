#pragma once
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	// Auxiliary class that helps to avoid creating multiple identical samplers
	// by maintaining a cache of already created ones
	class TextureSamplersCache
	{
	public:
		void Initialize(RenderDevice* pDevice);

		// Creates a new sampler or returns an existing one with the given configuration
		SamplerHandle CreateSampler(SamplerDesc desc);

		// Deletes all of the existing samplers in the cache
		void ClearCache();

	private:
		struct DescSamplerPair
		{
			SamplerDesc   m_Desc;
			SamplerHandle m_SamplerHandle;
		};

		RenderDevice*           m_pDevice{};
		Vector<DescSamplerPair> m_CachedSamplers{};
	};
}
