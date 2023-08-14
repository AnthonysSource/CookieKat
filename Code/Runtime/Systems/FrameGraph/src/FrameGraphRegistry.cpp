#include "CookieKat/Systems/FrameGraph/FrameGraphRegistry.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

namespace CKE {
	TextureViewHandle ExecuteResourcesCtx::GetTextureView(FGResourceID texture) {
		CKE_ASSERT(m_TextureViews.contains(texture));
		return m_TextureViews[texture];
	}

	TextureHandle ExecuteResourcesCtx::GetTexture(FGResourceID texture) {
		CKE_ASSERT(m_Textures.contains(texture));
		return m_Textures[texture];
	}

	BufferHandle ExecuteResourcesCtx::GetBuffer(FGResourceID buffer) {
		CKE_ASSERT(m_Buffers.contains(buffer));
		return m_Buffers[buffer];
	}

	void ExecuteResourcesCtx::RefreshContext(FrameGraphDB* pDb) {
		// We currently rebuild all of the context resource references
		// in each iteration because imported resource id's could change
		// (Like the swapchain textureID)
		for (FGPassResourceUsageInfo& usageMeta : m_ResourceUsages) {
			switch (usageMeta.m_Type) {
			case FGResourceType::Texture: {
				FGTextureData* pTex = pDb->GetTexture(usageMeta.m_ID);
				CKE_ASSERT(pTex->m_TexHandle.IsNotNull());
				CKE_ASSERT(pTex->m_ViewHandle.IsNotNull());
				m_Textures[usageMeta.m_ID] = pTex->m_TexHandle;
				m_TextureViews[usageMeta.m_ID] = pTex->m_ViewHandle;
			}
			break;
			case FGResourceType::Buffer: {
				BufferHandle bufferHandle = pDb->GetBuffer(usageMeta.m_ID)->m_Handle;
				m_Buffers[usageMeta.m_ID] = bufferHandle;
			}
			break;
			default: CKE_UNREACHABLE_CODE();
			}
		}
	}

	void ExecuteResourcesCtx::PopulateContext(Vector<FGPassResourceUsageInfo>& resources, FrameGraphDB* pDb) {
		m_ResourceUsages = resources;
		for (FGPassResourceUsageInfo& usageMeta : resources) {
			switch (usageMeta.m_Type) {
			case FGResourceType::Texture: {
				FGTextureData* pTex = pDb->GetTexture(usageMeta.m_ID);
				CKE_ASSERT(pTex->m_TexHandle.IsNotNull());
				CKE_ASSERT(pTex->m_ViewHandle.IsNotNull());
				m_Textures.insert({usageMeta.m_ID, pTex->m_TexHandle});
				m_TextureViews.insert({usageMeta.m_ID, pTex->m_ViewHandle});
			}
			break;
			case FGResourceType::Buffer: {
				BufferHandle bufferHandle = pDb->GetBuffer(usageMeta.m_ID)->m_Handle;
				m_Buffers.insert({usageMeta.m_ID, bufferHandle});
			}
			break;
			default: CKE_UNREACHABLE_CODE();
			}
		}
	}
}
