#pragma once
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class TextureSamplersCache;

	// Object-oriented interface over a texture handle and render device.
	class TextureRef
	{
	public:
		TextureRef(RenderDevice*   pDevice, TextureSamplersCache* pSamplersCache, TextureDesc texDesc,
		           TextureViewDesc viewDesc, SamplerDesc          samplerDesc) {
			m_pDevice = pDevice;
			m_pSamplerCache = pSamplersCache;
			m_TextureDesc = texDesc;
			m_TextureViewDesc = viewDesc;
			m_SamplerDesc = samplerDesc;

			m_Texture = pDevice->CreateTexture(texDesc);
			m_TextureFullView = pDevice->CreateTextureView(viewDesc);
			m_Sampler = m_pSamplerCache->CreateSampler(samplerDesc);
		}

		inline TextureHandle     GetTextureHandle() { return m_Texture; }
		inline TextureViewHandle GetFullView() { return m_TextureFullView; }
		inline SamplerHandle     GetDefaultSampler() { return m_Sampler; }

		inline void GenerateMips() {}

	private:
		RenderDevice*         m_pDevice{nullptr};
		TextureSamplersCache* m_pSamplerCache{nullptr};
		TextureDesc           m_TextureDesc{};
		TextureHandle         m_Texture{};
		TextureViewDesc       m_TextureViewDesc{};
		TextureViewHandle     m_TextureFullView{};
		SamplerDesc           m_SamplerDesc{};
		SamplerHandle         m_Sampler{};
	};
}
