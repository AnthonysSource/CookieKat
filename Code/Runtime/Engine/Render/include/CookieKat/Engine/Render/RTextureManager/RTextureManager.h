#pragma once

#include <CookieKat/Core/Platform/PrimitiveTypes.h>
#include "CookieKat/Core/Math/Math.h"

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"

namespace CKE {
	using RTextureHandle = u64;

	// Texture with a size relative to the BackBuffer size.
	// Automatically resizes if the BackBuffer size changes.
	class RTexture
	{
	public:
		// Retrieve the actual TextureHandle for this texture.
		// DO NOT save anywhere because it might change when
		// resizing the BackBuffer for example.
		TextureHandle GetHandle();
		TextureViewHandle GetViewHandle();
		TextureDesc GetDescription();

	private:
		friend class RTextureManager;

		RTextureHandle   m_RelHandle{};
		RTextureManager* m_pManager{nullptr};
	};

	// Manages creating textures with a size relative to the BackBuffer dimensions.
	// These textures get automatically resized when the BackBuffer changes
	class RTextureManager
	{
	public:
		void Initialize(RenderDevice* pDevice);
		void UpdateRenderTarget(Int2 newSize);

		// Create a texture with relative size
		RTextureHandle CreateRTexture(TextureDesc desc);

		// Create a new relative size texture and return a
		// texture object that can retrieve the actual TextureHandle
		// without an explicit reference to the manager.
		// This is just for easier use, so you don't have to maintain
		// a reference to the manager to be able to resolve the handles
		RTexture CreateRTextureObj(TextureDesc desc);
		void     RemoveRTexture(RTextureHandle texHandle);

		// Return the actual TextureHandle of a given RelativeTexture Handle
		TextureHandle GetUnderlyingTextureHandle(RTextureHandle relHandle);
		// Return the full Texture View of a given RelativeTexture Handle
		TextureViewHandle GetTextureViewHandle(RTextureHandle relHandle);
		TextureDesc GetTextureDesc(RTextureHandle relHandle);

	private:
		RTextureHandle GenerateHandle();

	private:
		RenderDevice* m_pDevice{nullptr};

		struct RTTexture
		{
			TextureDesc       m_TexDesc;
			TextureHandle     m_TexHandle;
			TextureViewHandle m_FullTexViewHandle;
		};

		Map<RTextureHandle, RTTexture> m_RTTextures{};
		RTextureHandle                 m_LastHandle = 0;
	};
}
