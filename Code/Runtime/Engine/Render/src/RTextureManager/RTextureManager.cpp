#include "CookieKat/Engine/Render/RTextureManager/RTextureManager.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	void RTextureManager::Initialize(RenderDevice* pDevice) {
		CKE_ASSERT(pDevice != nullptr);
		m_pDevice = pDevice;
	}

	void RTextureManager::UpdateRenderTarget(Int2 newSize) {
		for (auto& [id, texHandle] : m_RTTextures) {
			m_pDevice->DestroyTexture(texHandle.m_TexHandle);

			TextureDesc& desc = texHandle.m_TexDesc;
			desc.m_Size = {newSize.x, newSize.y, desc.m_Size.z};
			texHandle.m_TexHandle = m_pDevice->CreateTexture(desc);

			TextureViewDesc viewDesc{};
			viewDesc.m_Format = desc.m_Format;
			viewDesc.m_Type = TextureViewType::Tex2D;
			viewDesc.m_AspectMask = desc.m_AspectMask;
			viewDesc.m_Texture = texHandle.m_TexHandle;
			texHandle.m_FullTexViewHandle = m_pDevice->CreateTextureView(viewDesc);
		}
	}

	RTextureHandle RTextureManager::CreateRTexture(TextureDesc desc) {
		// Create Device Texture
		TextureHandle   texHandle = m_pDevice->CreateTexture(desc);
		TextureViewDesc viewDesc{};
		viewDesc.m_Format = desc.m_Format;
		viewDesc.m_Type = TextureViewType::Tex2D;
		viewDesc.m_AspectMask = desc.m_AspectMask;
		viewDesc.m_Texture = texHandle;
		TextureViewHandle viewHandle = m_pDevice->CreateTextureView(viewDesc);

		// Setup internal structures to keep track of it and update it
		RTextureHandle relHandle = GenerateHandle();
		RTTexture      rtTexture{};
		rtTexture.m_TexDesc = desc;
		rtTexture.m_TexHandle = texHandle;
		rtTexture.m_FullTexViewHandle = viewHandle;
		m_RTTextures.insert({relHandle, rtTexture});

		return relHandle;
	}

	RTexture RTextureManager::CreateRTextureObj(TextureDesc desc) {
		RTexture t{};
		t.m_pManager = this;
		t.m_RelHandle = CreateRTexture(desc);
		return t;
	}

	void RTextureManager::RemoveRTexture(RTextureHandle texHandle) {
		// TODO: Implement this pls
	}

	TextureHandle RTextureManager::GetUnderlyingTextureHandle(RTextureHandle relHandle) {
		CKE_ASSERT(m_RTTextures.contains(relHandle));
		return m_RTTextures[relHandle].m_TexHandle;
	}

	TextureViewHandle RTextureManager::GetTextureViewHandle(RTextureHandle relHandle) {
		CKE_ASSERT(m_RTTextures.contains(relHandle));
		return m_RTTextures[relHandle].m_FullTexViewHandle;
	}

	TextureDesc RTextureManager::GetTextureDesc(RTextureHandle relHandle) {
		CKE_ASSERT(m_RTTextures.contains(relHandle));
		return m_RTTextures[relHandle].m_TexDesc;
	}

	RTextureHandle RTextureManager::GenerateHandle() {
		m_LastHandle++;
		return m_LastHandle;
	}

	TextureHandle RTexture::GetHandle() {
		return m_pManager->GetUnderlyingTextureHandle(m_RelHandle);
	}

	TextureViewHandle RTexture::GetViewHandle() {
		return m_pManager->GetTextureViewHandle(m_RelHandle);
	}

	TextureDesc RTexture::GetDescription() {
		return m_pManager->GetTextureDesc(m_RelHandle);
	}
}
