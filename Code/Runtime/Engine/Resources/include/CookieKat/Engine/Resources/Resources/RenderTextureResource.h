#pragma once

#include "CookieKat/Systems/Resources/IResource.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	class TextureLoader;
	class CubeMapLoader;
	class ResourceCompiler;
	class TextureCompiler;
	class CubeMapCompiler;
}

namespace CKE {
	class RenderCubeMapResource : public IResource
	{
		CKE_SERIALIZE(m_FaceWidth, m_FaceHeight, m_Faces);

	public:
		TextureHandle     GetTexture() const { return m_Texture; }
		TextureViewHandle GetTextureView() const { return m_TextureView; }
		Array<Vector<u8>, 6> const& GetFaces() const { return m_Faces; }
		u32 GetFaceSize() const { return m_FaceWidth; }

	private:
		friend CubeMapLoader;
		friend ResourceCompiler;
		friend CubeMapCompiler;

	private:
		TextureHandle        m_Texture;
		TextureViewHandle    m_TextureView;
		u32                  m_FaceWidth{};
		u32                  m_FaceHeight{};
		Array<Vector<u8>, 6> m_Faces{};
	};
}

namespace CKE {
	class RenderTextureResource : public IResource
	{
		CKE_SERIALIZE(m_Data, m_Desc);

		friend TextureLoader;
		friend ResourceCompiler;
		friend TextureCompiler;

	public:
		inline TextureHandle const&     GetTexture() const { return m_TextureHandle; }
		inline TextureViewHandle const& GetTextureView() const { return m_TextureView; }
		inline TextureDesc const&       GetTextureDesc() const { return m_Desc; }

	private:
		Blob        m_Data{};
		TextureDesc m_Desc{};

		TextureHandle     m_TextureHandle{};
		TextureViewHandle m_TextureView{};
	};
}
