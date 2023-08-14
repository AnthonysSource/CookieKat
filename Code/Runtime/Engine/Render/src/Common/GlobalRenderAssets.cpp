#include "Common/GlobalRenderAssets.h"

#include "Common/BasicMeshDefinitions.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/RenderUtils/TextureUploader.h"

namespace CKE {
	void GlobalRenderAssets::Initialize(RenderDevice& m_Device) {
		TextureDesc textureDesc{};
		textureDesc.m_Size = {1, 1, 1};
		textureDesc.m_AspectMask = TextureAspectMask::Color;
		textureDesc.m_TextureType = TextureType::Tex2D;
		textureDesc.m_Usage = TextureUsage::Sampled | TextureUsage::Transfer_Dst;

		TextureViewDesc viewDesc{};
		viewDesc.m_Type = TextureViewType::Tex2D;
		viewDesc.m_AspectMask = TextureAspectMask::Color;

		TextureUploader uploader{};
		uploader.Initialize(&m_Device, sizeof(u32));

		u8 whiteCol[4] = {255, 255, 255, 255};
		textureDesc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		m_White1x1 = m_Device.CreateTexture(textureDesc);
		uploader.UploadColorTexture2D(m_White1x1, &whiteCol, UInt2{1, 1}, sizeof(u32));
		viewDesc.m_Texture = m_White1x1;
		m_White1x1_View = m_Device.CreateTextureView(viewDesc);

		u8 blackCol[4] = {0, 0, 0, 255};
		textureDesc.m_Format = TextureFormat::R8G8B8A8_SRGB;
		m_Black1x1 = m_Device.CreateTexture(textureDesc);
		uploader.UploadColorTexture2D(m_Black1x1, &blackCol, UInt2{1, 1}, sizeof(u32));
		viewDesc.m_Texture = m_Black1x1;
		m_Black1x1_View = m_Device.CreateTextureView(viewDesc);

		u8 normalDefault[4] = {128, 128, 255, 255};
		textureDesc.m_Format = TextureFormat::R8G8B8A8_UNORM;
		m_NormalDefault = m_Device.CreateTexture(textureDesc);
		uploader.UploadColorTexture2D(m_NormalDefault, &normalDefault, UInt2{1, 1}, sizeof(u32));
		viewDesc.m_Format = TextureFormat::R8G8B8A8_UNORM;
		viewDesc.m_Texture = m_NormalDefault;
		m_NormalDefault_View = m_Device.CreateTextureView(viewDesc);

		uploader.Shutdown();

		ScreenQuad        screenQuad{};
		BufferDesc screenQuadBufferDesc{};
		screenQuadBufferDesc.m_Name = "ScreenQuad Buffer";
		screenQuadBufferDesc.m_Usage = BufferUsage::Vertex | BufferUsage::TransferDst;
		screenQuadBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		screenQuadBufferDesc.m_SizeInBytes = sizeof(ScreenQuad);
		screenQuadBufferDesc.m_StrideInBytes = sizeof(screenQuad.m_Verts[0]);
		m_ScreenQuad = m_Device.CreateBuffer_DEPR(screenQuadBufferDesc, &screenQuad.m_Verts, sizeof(screenQuad.m_Verts));

		CubeMapMesh       cube{};
		BufferDesc cubeBufferDesc{};
		cubeBufferDesc.m_Name = "CubeMapMesh Buffer";
		cubeBufferDesc.m_Usage = BufferUsage::Vertex | BufferUsage::TransferDst;
		cubeBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		cubeBufferDesc.m_SizeInBytes = sizeof(CubeMapMesh);
		cubeBufferDesc.m_StrideInBytes = sizeof(cube.m_Verts[0]) * 3;
		m_CubeMapMesh = m_Device.CreateBuffer_DEPR(cubeBufferDesc, &cube.m_Verts, sizeof(cube.m_Verts));
	}

	void GlobalRenderAssets::Shutdown(RenderDevice& device) {
		device.DestroyTexture(m_White1x1);
		device.DestroyTexture(m_Black1x1);
		device.DestroyTexture(m_NormalDefault);
		device.DestroyBuffer(m_CubeMapMesh);
		device.DestroyBuffer(m_ScreenQuad);
	}
}
