#pragma once

#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"

namespace CKE {
	// Available context for a pass when in the execution phase
	class ExecuteResourcesCtx
	{
	public:
		// Returns a complete view of the texture associated with the given ID
		// Asserts:
		//   Texture is available for pass
		TextureViewHandle GetTextureView(FGResourceID texture);

		// Returns the texture associated with the given ID
		// Asserts:
		//   Texture is available for pass
		TextureHandle GetTexture(FGResourceID texture);

		// Returns the buffer associated with the given ID
		// Asserts:
		//   Buffer is available for pass
		BufferHandle GetBuffer(FGResourceID buffer);

	private:
		friend class FrameGraph;
		friend class FrameGraphDB;

		void RefreshContext(FrameGraphDB* pDb);
		void PopulateContext(Vector<FGPassResourceUsageInfo>& resources, FrameGraphDB* pDb);

		Vector<FGPassResourceUsageInfo>      m_ResourceUsages;
		Map<FGResourceID, TextureHandle>     m_Textures;
		Map<FGResourceID, TextureViewHandle> m_TextureViews;
		Map<FGResourceID, BufferHandle>      m_Buffers;
	};
}
