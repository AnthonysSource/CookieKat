#pragma once

#include "RenderTextureResource.h"

#include "CookieKat/Core/Serialization/Archive.h"
#include "CookieKat/Systems/Resources/ResourceID.h"

namespace CKE {
	class RenderMaterialResource : public IResource
	{
		CKE_SERIALIZE(m_AlbedoTexture, m_RoughnessTexture, m_MetalicTexture, m_NormalTexture);

		friend class MaterialLoader;
		friend class ResourceCompiler;

	public:
		inline TResourceID<RenderTextureResource> const& GetAlbedoTexture() const;
		inline TResourceID<RenderTextureResource> const& GetRoughnessTexture() const;
		inline TResourceID<RenderTextureResource> const& GetMetalicTexture() const;
		inline TResourceID<RenderTextureResource> const& GetNormalTexture() const;

	private:
		TResourceID<RenderTextureResource> m_AlbedoTexture;
		TResourceID<RenderTextureResource> m_RoughnessTexture;
		TResourceID<RenderTextureResource> m_MetalicTexture;
		TResourceID<RenderTextureResource> m_NormalTexture;
	};
}

namespace CKE {
	inline TResourceID<RenderTextureResource> const& RenderMaterialResource::GetAlbedoTexture() const {
		return m_AlbedoTexture;
	}

	inline TResourceID<RenderTextureResource> const& RenderMaterialResource::
	GetRoughnessTexture() const { return m_RoughnessTexture; }

	inline TResourceID<RenderTextureResource> const& RenderMaterialResource::GetMetalicTexture() const {
		return m_MetalicTexture;
	}

	inline TResourceID<RenderTextureResource> const& RenderMaterialResource::GetNormalTexture() const {
		return m_NormalTexture;
	}
}
