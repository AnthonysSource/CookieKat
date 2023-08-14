#include "Loaders/TextureLoader.h"

#include "CookieKat/Core/Memory/Memory.h"
#include "CookieKat/Engine/Resources/Resources/RenderTextureResource.h"

#include <lodepng.h>

namespace CKE {
	void TextureLoader::Initialize(RenderDevice* pRenderDevice) {
		m_pRenderDevice = pRenderDevice;
		m_Uploader.Initialize(m_pRenderDevice, sizeof(u32) * 4 * 4096 * 4096);
	}

	void TextureLoader::Shutdown() {
		m_Uploader.Shutdown();
	}

	LoadResult TextureLoader::LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) {
		auto pTexture = New<RenderTextureResource>();
		ar << *pTexture;

		UInt3 texSize = pTexture->m_Desc.m_Size;

		// Uncompress Texture
		Vector<u8> uncompressedTexture{};
		uncompressedTexture.reserve(texSize.x * texSize.y);
		lodepng::decode(uncompressedTexture, texSize.x, texSize.y, pTexture->m_Data);
		pTexture->m_Data = uncompressedTexture;

		out.SetResource(pTexture);
		return LoadResult::Successful;
	}

	LoadResult TextureLoader::Install(InstallContext& ctx) {
		auto pTexture = ctx.GetResource<RenderTextureResource>();

		UInt3 texSize = pTexture->m_Desc.m_Size;

		// Allocate and create texture in GPU
		TextureDesc texDesc{};
		texDesc.m_Size = UInt3{texSize.x, texSize.y, texSize.z};
		texDesc.m_AspectMask = TextureAspectMask::Color;
		texDesc.m_TextureType = TextureType::Tex2D;
		texDesc.m_Format = pTexture->m_Desc.m_Format;
		TextureHandle texHandle = m_pRenderDevice->CreateTexture(texDesc);
		pTexture->m_TextureHandle = texHandle;

		// Upload it to GPU
		CKE_ASSERT(texSize.x <= 4096 && texSize.y <= 4096);
		m_Uploader.UploadColorTexture2D(texHandle, pTexture->m_Data.data(),
		                              UInt2{texSize.x, texSize.y}, sizeof(u32));

		// Create Texture View of the complete texture
		TextureViewDesc viewDesc{};
		viewDesc.m_Format = texDesc.m_Format;
		viewDesc.m_Texture = texHandle;
		viewDesc.m_Type = TextureViewType::Tex2D;
		viewDesc.m_AspectMask = TextureAspectMask::Color;
		pTexture->m_TextureView = m_pRenderDevice->CreateTextureView(viewDesc);

		return LoadResult::Successful;
	}

	LoadResult TextureLoader::Uninstall(UninstallContext& ctx) {
		RenderTextureResource* pTex = ctx.GetResource<RenderTextureResource>();
		m_pRenderDevice->DestroyTextureView(pTex->m_TextureView);
		m_pRenderDevice->DestroyTexture(pTex->m_TextureHandle);
		return LoadResult::Successful;
	}

	LoadResult TextureLoader::Unload(UnloadContext& ctx) {
		auto pTex = ctx.GetResource<RenderTextureResource>();
		CKE::Delete(pTex);
		return LoadResult::Successful;
	}
}

namespace CKE {
	void CubeMapLoader::Initialize(RenderDevice* pRenderDevice) {
		CKE_ASSERT(pRenderDevice != nullptr);
		m_pRenderDevice = pRenderDevice;
	}

	LoadResult CubeMapLoader::LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) {
		auto pCubeMapAsset = CKE::New<RenderCubeMapResource>();
		ar << *pCubeMapAsset;
		out.SetResource(pCubeMapAsset);
		return LoadResult::Successful;
	}

	LoadResult CubeMapLoader::Install(InstallContext& ctx) {
		auto cubeMap = ctx.GetResource<RenderCubeMapResource>();

		Array<Vector<u8>, 6> rawTex{};
		void*                pCubeMapPtrs[6];
		for (int i = 0; i < 6; ++i) {
			rawTex[i].reserve(cubeMap->m_FaceWidth * cubeMap->m_FaceHeight);
			lodepng::decode(rawTex[i],
			                cubeMap->m_FaceWidth,
			                cubeMap->m_FaceHeight,
			                cubeMap->m_Faces[i]);
			pCubeMapPtrs[i] = rawTex[i].data();
		}
		cubeMap->m_Faces = rawTex;

		TextureDesc textureDesc{};
		textureDesc.m_Name = "CubeMap";
		textureDesc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		textureDesc.m_Size = UInt3{cubeMap->m_FaceWidth, cubeMap->m_FaceHeight, 1};
		textureDesc.m_AspectMask = TextureAspectMask::Color;
		textureDesc.m_MiscFlags = TextureMiscFlags::Texture_CubeMap;
		textureDesc.m_ArraySize = 6;
		cubeMap->m_Texture = m_pRenderDevice->CreateTexture(textureDesc);

		TextureUploader uploader{};
		uploader.Initialize(m_pRenderDevice, cubeMap->m_FaceWidth * cubeMap->m_FaceHeight * 4 * 6);
		uploader.UploadTextureCubeMap(cubeMap->m_Texture, pCubeMapPtrs,
		                              UInt2{cubeMap->m_FaceWidth, cubeMap->m_FaceWidth});
		uploader.Shutdown();

		TextureViewDesc viewDesc{
			cubeMap->m_Texture, TextureViewType::Cube,
			textureDesc.m_AspectMask, textureDesc.m_Format,
			0, 1, 0, 6
		};
		cubeMap->m_TextureView = m_pRenderDevice->CreateTextureView(viewDesc);

		return LoadResult::Successful;
	}

	LoadResult CubeMapLoader::Uninstall(UninstallContext& ctx) {
		auto cubeMap = ctx.GetResource<RenderCubeMapResource>();
		m_pRenderDevice->DestroyTextureView(cubeMap->m_TextureView);
		m_pRenderDevice->DestroyTexture(cubeMap->m_Texture);

		return LoadResult::Successful;
	}

	LoadResult CubeMapLoader::Unload(UnloadContext& ctx) {
		auto pCubeMap = ctx.GetResource<RenderCubeMapResource>();
		CKE::Delete(pCubeMap);
		return LoadResult::Successful;
	}
}
