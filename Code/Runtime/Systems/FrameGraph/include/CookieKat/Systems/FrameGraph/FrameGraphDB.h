#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"

namespace CKE {
	class FrameGraphDB
	{
	public:
		void AddTransientTexture(FGResourceID      fgID, TextureHandle              texHandle, TextureDesc desc,
		                         TextureViewHandle viewHandle, TextureExtraSettings extra);
		void AddImportedTexture(FGImportedTextureDesc desc);

		void AddTransientBuffer(FGResourceID fgID, BufferHandle handle, BufferDesc desc);
		void AddImportedBuffer(FGResourceID fgID, BufferHandle handle, BufferDesc desc);

		void RemoveTransientTexture(FGResourceID id);
		void RemoveTransientBuffer(FGResourceID id);

		FGTextureData*      GetTexture(FGResourceID fgID);
		FGTransientTexture* GetTransientTexture(FGResourceID fgID);
		FGImportedTexture*  GetImportedTexture(FGResourceID fgID);

		FGBufferData*      GetBuffer(FGResourceID fgID);
		FGTransientBuffer* GetTransientBuffer(FGResourceID fgID);
		FGImportedBuffer*  GetImportedBuffer(FGResourceID fgID);

		Vector<FGResourceID> GetAllTransientTextures();
		Vector<FGResourceID> GetAllImportedTextures();

		Vector<FGResourceID> GetAllTransientBuffers();

		bool CheckImportedTextureExists(FGResourceID fgID);

	private:
		Map<FGResourceID, FGResourceInfo> m_ResourceMetadata;

		Map<FGResourceID, FGTextureData> m_Textures;
		Map<FGResourceID, FGBufferData>  m_Buffers;

		Map<FGResourceID, FGTransientTexture> m_TransientTextures;
		Map<FGResourceID, FGTransientBuffer>  m_TransientBuffers;

		Map<FGResourceID, FGImportedTexture> m_ImportedTextures;
		Map<FGResourceID, FGImportedBuffer>  m_ImportedBuffers;
	};
}
