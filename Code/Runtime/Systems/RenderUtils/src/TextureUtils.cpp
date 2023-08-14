#include "CookieKat/Systems/RenderAPI/Vulkan/Conversions_Vk.h"
#include "CookieKat/Systems/RenderUtils/TextureUploader.h"
#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"

namespace CKE {
	void TextureUploader::Initialize(RenderDevice* pDevice, u32 stagingBufferSize) {
		// Create and fill staging buffer
		BufferDesc stagingBufferDesc{};
		stagingBufferDesc.m_Usage = BufferUsage::TransferSrc;
		stagingBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		stagingBufferDesc.m_SizeInBytes = stagingBufferSize;
		m_StagingBuffer = pDevice->CreateBuffer(stagingBufferDesc);

		m_pDevice = pDevice;
		m_StagingBufferSize = stagingBufferSize;
	}

	void TextureUploader::Shutdown() {
		m_pDevice->DestroyBuffer(m_StagingBuffer);
	}

	void TextureUploader::UploadColorTexture2D(TextureHandle targetTexture, void* pTextureData,
	                                           UInt2         texSize,
	                                           u32           pixelByteSize) {
		UploadTexture2D(targetTexture, pTextureData, texSize, pixelByteSize,
		                TextureAspectMask::Color);
	}

	void TextureUploader::UploadTexture2D(TextureHandle targetTexture, void* pTextureData,
	                                      UInt2         texSize,
	                                      u32           pixelByteSize, TextureAspectMask aspectType) {
		CKE_ASSERT(pTextureData != nullptr);
		u32 textureByteSize = texSize.x * texSize.y * pixelByteSize;
		CKE_ASSERT(textureByteSize <= m_StagingBufferSize);

		m_pDevice->UploadBufferData_DEPR(m_StagingBuffer, pTextureData, textureByteSize, 0);

		// Set image layout to Transfer Dst
		//-----------------------------------------------------------------------------

		GraphicsCommandList graphicsCmdList = m_pDevice->GetGraphicsCmdList();

		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::AllCommands,
			.m_SrcAccessMask = AccessMask::None,
			.m_DstStage = PipelineStage::Transfer,
			.m_DstAccessMask = AccessMask::Transfer_Write,
			.m_OldLayout = TextureLayout::Undefined,
			.m_NewLayout = TextureLayout::Transfer_Dst,
			.m_Texture = targetTexture,
			.m_AspectMask = aspectType,
		});
		graphicsCmdList.End();

		SemaphoreHandle imageLayoutToTransfer = m_pDevice->CreateSemaphoreGPU();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {
			                                     .m_SignalSemaphores = {imageLayoutToTransfer}
		                                     });

		// Copy data from staging buffer to image
		//-----------------------------------------------------------------------------

		TransferCommandList transferCtx = m_pDevice->GetTransferCmdList();

		VkBufferImageCopy copyRegion{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = {
				.aspectMask = ConversionsVK::GetVkImageAspectFlags(aspectType),
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = {0, 0, 0},
			.imageExtent = VkExtent3D{texSize.x, texSize.y, 1},
		};

		transferCtx.Begin();
		transferCtx.CopyBufferToTexture(m_StagingBuffer, targetTexture, copyRegion);
		transferCtx.End();
		FenceHandle const transferFinished = m_pDevice->CreateFence(false);
		m_pDevice->SubmitTransferCommandList(transferCtx, {
			                                     .m_WaitSemaphores = {
				                                     {
					                                     imageLayoutToTransfer,
					                                     PipelineStage::Transfer
				                                     }
			                                     },
			                                     .m_SignalSemaphores = {},
			                                     .m_SignalFence = transferFinished
		                                     });

		// Set image layout to shader read only
		//-----------------------------------------------------------------------------

		m_pDevice->WaitForFence(transferFinished);
		m_pDevice->ResetFence(transferFinished);
		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::Transfer,
			.m_SrcAccessMask = AccessMask::Transfer_Write,
			.m_DstStage = PipelineStage::AllCommands,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Transfer_Dst,
			.m_NewLayout = TextureLayout::Shader_ReadOnly,
			.m_Texture = targetTexture,
			.m_AspectMask = aspectType,
		});
		graphicsCmdList.End();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {.m_SignalFence = transferFinished});
		m_pDevice->WaitForFence(transferFinished);

		m_pDevice->DestroyFence(transferFinished);
		m_pDevice->DestroySemaphore(imageLayoutToTransfer);
	}

	void TextureUploader::UploadTextureCubeMap(TextureHandle targetTexture, void* pTextureData,
	                                           UInt2         texFaceSize) {
		CKE_ASSERT(pTextureData != nullptr);
		// Assume a pixel size of 32 bits
		constexpr u32 pixelByteSize = 4;
		u32           textureByteSize = texFaceSize.x * texFaceSize.y * pixelByteSize * 6;
		CKE_ASSERT(textureByteSize <= m_StagingBufferSize);

		m_pDevice->UploadBufferData_DEPR(m_StagingBuffer, pTextureData, textureByteSize, 0);

		// Set image layout to Transfer Dst
		//-----------------------------------------------------------------------------

		GraphicsCommandList graphicsCmdList = m_pDevice->GetGraphicsCmdList();
		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::AllCommands,
			.m_SrcAccessMask = AccessMask::None,
			.m_DstStage = PipelineStage::Transfer,
			.m_DstAccessMask = AccessMask::Transfer_Write,
			.m_OldLayout = TextureLayout::Undefined,
			.m_NewLayout = TextureLayout::Transfer_Dst,
			.m_Texture = targetTexture,
			.m_AspectMask = TextureAspectMask::Color,
			.m_Range = TextureRange{
				.m_AspectMask = TextureAspectMask::Color,
				.m_BaseMip = 0,
				.m_MipCount = 1,
				.m_BaseLayer = 0,
				.m_LayerCount = 6
			}
		});
		graphicsCmdList.End();
		SemaphoreHandle imageLayoutTransfer = m_pDevice->CreateSemaphoreGPU();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {
			                                     .m_SignalSemaphores = {imageLayoutTransfer}
		                                     });

		// Copy data from staging buffer to image
		//-----------------------------------------------------------------------------

		TransferCommandList transferCtx = m_pDevice->GetTransferCmdList();

		transferCtx.Begin();
		for (i32 i = 0; i < 6; ++i) {
			VkBufferImageCopy copyRegion{
				.bufferOffset = texFaceSize.x * i * pixelByteSize,
				.bufferRowLength = texFaceSize.x * 6,
				.bufferImageHeight = texFaceSize.y,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = (u32)i,
					.layerCount = 1,
				},
				.imageOffset = {0, 0, 0},
				.imageExtent = VkExtent3D{texFaceSize.x, texFaceSize.y, 1},
			};

			transferCtx.CopyBufferToTexture(m_StagingBuffer, targetTexture, copyRegion);
		}
		transferCtx.End();
		FenceHandle const transferFinished = m_pDevice->CreateFence(false);
		m_pDevice->SubmitTransferCommandList(transferCtx, {
			                                     .m_WaitSemaphores = {
				                                     {imageLayoutTransfer, PipelineStage::Transfer}
			                                     },
			                                     .m_SignalSemaphores = {},
			                                     .m_SignalFence = transferFinished
		                                     });

		// Set image layout to shader read only
		//-----------------------------------------------------------------------------

		m_pDevice->WaitForFence(transferFinished);
		m_pDevice->ResetFence(transferFinished);
		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::Transfer,
			.m_SrcAccessMask = AccessMask::Transfer_Write,
			.m_DstStage = PipelineStage::AllCommands,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Transfer_Dst,
			.m_NewLayout = TextureLayout::Shader_ReadOnly,
			.m_Texture = targetTexture,
			.m_AspectMask = TextureAspectMask::Color,
			.m_Range = TextureRange{
				.m_AspectMask = TextureAspectMask::Color,
				.m_BaseMip = 0,
				.m_MipCount = 1,
				.m_BaseLayer = 0,
				.m_LayerCount = 6
			}
		});
		graphicsCmdList.End();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {.m_SignalFence = transferFinished});
		m_pDevice->WaitForFence(transferFinished);

		m_pDevice->DestroyFence(transferFinished);
		m_pDevice->DestroySemaphore(imageLayoutTransfer);
	}

	void TextureUploader::UploadTextureCubeMap(TextureHandle targetTexture, void* pTexFaceData[6],
	                                           UInt2         texFaceSize) {
		CKE_ASSERT(pTexFaceData != nullptr);
		// Assume a pixel size of 32 bits
		constexpr u32 pixelByteSize = 4;
		u32           faceByteSize = texFaceSize.x * texFaceSize.y * pixelByteSize;
		u32           textureByteSize = faceByteSize * 6;
		CKE_ASSERT(textureByteSize <= m_StagingBufferSize);

		for (int i = 0; i < 6; ++i) {
			m_pDevice->UploadBufferData_DEPR(m_StagingBuffer, pTexFaceData[i], faceByteSize,
			                                 faceByteSize * i);
		}

		// Set image layout to Transfer Dst
		//-----------------------------------------------------------------------------

		GraphicsCommandList graphicsCmdList = m_pDevice->GetGraphicsCmdList();
		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::AllCommands,
			.m_SrcAccessMask = AccessMask::None,
			.m_DstStage = PipelineStage::Transfer,
			.m_DstAccessMask = AccessMask::Transfer_Write,
			.m_OldLayout = TextureLayout::Undefined,
			.m_NewLayout = TextureLayout::Transfer_Dst,
			.m_Texture = targetTexture,
			.m_AspectMask = TextureAspectMask::Color,
			.m_Range = TextureRange{
				.m_AspectMask = TextureAspectMask::Color,
				.m_BaseMip = 0,
				.m_MipCount = 1,
				.m_BaseLayer = 0,
				.m_LayerCount = 6
			}
		});
		graphicsCmdList.End();
		SemaphoreHandle imageLayoutTransfer = m_pDevice->CreateSemaphoreGPU();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {
			                                     .m_SignalSemaphores = {imageLayoutTransfer}
		                                     });

		// Copy data from staging buffer to image
		//-----------------------------------------------------------------------------

		TransferCommandList transferCtx = m_pDevice->GetTransferCmdList();

		transferCtx.Begin();
		for (i32 i = 0; i < 6; ++i) {
			VkBufferImageCopy copyRegion{
				.bufferOffset = texFaceSize.x * texFaceSize.y * pixelByteSize * i,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = (u32)i,
					.layerCount = 1,
				},
				.imageOffset = {0, 0, 0},
				.imageExtent = VkExtent3D{texFaceSize.x, texFaceSize.y, 1},
			};

			transferCtx.CopyBufferToTexture(m_StagingBuffer, targetTexture, copyRegion);
		}
		transferCtx.End();
		FenceHandle const transferFinished = m_pDevice->CreateFence(false);
		m_pDevice->SubmitTransferCommandList(transferCtx, {
			                                     .m_WaitSemaphores = {
				                                     {imageLayoutTransfer, PipelineStage::Transfer}
			                                     },
			                                     .m_SignalSemaphores = {},
			                                     .m_SignalFence = transferFinished
		                                     });

		// Set image layout to shader read only
		//-----------------------------------------------------------------------------

		m_pDevice->WaitForFence(transferFinished);
		m_pDevice->ResetFence(transferFinished);
		graphicsCmdList.Begin();
		graphicsCmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::Transfer,
			.m_SrcAccessMask = AccessMask::Transfer_Write,
			.m_DstStage = PipelineStage::AllCommands,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Transfer_Dst,
			.m_NewLayout = TextureLayout::Shader_ReadOnly,
			.m_Texture = targetTexture,
			.m_AspectMask = TextureAspectMask::Color,
			.m_Range = TextureRange{
				.m_AspectMask = TextureAspectMask::Color,
				.m_BaseMip = 0,
				.m_MipCount = 1,
				.m_BaseLayer = 0,
				.m_LayerCount = 6
			}
		});
		graphicsCmdList.End();
		m_pDevice->SubmitGraphicsCommandList(graphicsCmdList, {.m_SignalFence = transferFinished});
		m_pDevice->WaitForFence(transferFinished);

		m_pDevice->DestroyFence(transferFinished);
		m_pDevice->DestroySemaphore(imageLayoutTransfer);
	}
}

namespace CKE {
	void TextureSamplersCache::Initialize(RenderDevice* pDevice) {
		m_pDevice = pDevice;
	}

	SamplerHandle TextureSamplersCache::CreateSampler(SamplerDesc desc) {
		for (DescSamplerPair& pair : m_CachedSamplers) {
			if (pair.m_Desc.IsEqual(desc)) {
				return pair.m_SamplerHandle;
			}
		}

		SamplerHandle samplerHandle = m_pDevice->CreateSampler(desc);
		m_CachedSamplers.push_back(DescSamplerPair{
			desc,
			samplerHandle
		});
		return samplerHandle;
	}

	void TextureSamplersCache::ClearCache() {
		for (DescSamplerPair& pair : m_CachedSamplers) {
			m_pDevice->DestroySampler(pair.m_SamplerHandle);
		}
		m_CachedSamplers.clear();
	}
}
