#pragma once
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	// Auxiliary class that facilitates uploading different texture data to the GPU
	// Creates a staging buffer on initialization
	class TextureUploader
	{
	public:
		void Initialize(RenderDevice* pDevice, u32 stagingBufferSize);
		void Shutdown();

		void UploadColorTexture2D(TextureHandle targetTexture, void* pTextureData, UInt2 texSize, u32 pixelByteSize);
		void UploadTexture2D(TextureHandle     targetTexture, void* pTextureData, UInt2 texSize, u32 pixelByteSize,
			TextureAspectMask aspectType);
		void UploadTextureCubeMap(TextureHandle targetTexture, void* pTextureData, UInt2 texFaceSize);
		void UploadTextureCubeMap(TextureHandle targetTexture, void* pTexFaceData[6], UInt2 texFaceSize);

	private:
		BufferHandle  m_StagingBuffer{};
		RenderDevice* m_pDevice = nullptr;
		u32           m_StagingBufferSize{};
	};
}
