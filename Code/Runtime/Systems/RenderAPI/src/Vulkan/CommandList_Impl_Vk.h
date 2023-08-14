#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/RenderAPI/Internal/Swapchain.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/Instance_Vk.h"

#include "Vulkan/Conversions_Vk.h"

#include <iostream>
#include <ostream>

namespace CKE {
	void CommandList::Begin() {
		vkResetCommandBuffer(m_CmdBuffer, 0);

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// We re-record all of the command lists each frame so we can use this flag
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(m_CmdBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
			std::cout << "Error beginning to record command buffer" << std::endl;
		}
	}

	void CommandList::End() {
		if (vkEndCommandBuffer(m_CmdBuffer) != VK_SUCCESS) {
			std::cout << "Error ending command buffer" << std::endl;
		}
	}

	void CommandList::BeginDebugLabel(const char* pName, Vec3 color) {
		VkDebugUtilsLabelEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		info.pLabelName = pName;
		info.color[0] = color.x;
		info.color[1] = color.y;
		info.color[2] = color.z;
		info.color[3] = 1.0f;
		pfnCmdBeginDebugUtilsLabelEXT(m_CmdBuffer, &info);
	}

	void CommandList::EndDebugLabel() {
		pfnCmdEndDebugUtilsLabelEXT(m_CmdBuffer);
	}

	void GraphicsCommandList::BeginRendering(RenderingInfo renderingInfo) {
		Vector<VkRenderingAttachmentInfo> vkColAttach{};

		// Color
		for (RenderingAttachment& attach : renderingInfo.m_ColorAttachments) {
			vkColAttach.push_back(VkRenderingAttachmentInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = m_pDevice->m_ResourcesDB.GetTextureView(attach.m_TextureView)->m_vkView,
				.imageLayout = ConversionsVK::GetVkImageLayout(attach.m_Layout),
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.loadOp = ConversionsVK::GetVkLoadOp(attach.m_LoadOp),
				.storeOp = ConversionsVK::GetVkStoreOp(attach.m_StoreOp),
				.clearValue = VkClearValue{
					attach.m_ClearValue.x,
					attach.m_ClearValue.y,
					attach.m_ClearValue.z,
					attach.m_ClearValue.w
				}
			});
		}

		// Depth
		RenderingAttachment&      depthAttach = renderingInfo.m_DepthAttachment;
		VkRenderingAttachmentInfo vkDepthAttach{};
		if (renderingInfo.m_UseDepthAttachment) {
			vkDepthAttach = VkRenderingAttachmentInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = m_pDevice->m_ResourcesDB.GetTextureView(depthAttach.m_TextureView)->m_vkView,
				.imageLayout = ConversionsVK::GetVkImageLayout(depthAttach.m_Layout),
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.loadOp = ConversionsVK::GetVkLoadOp(depthAttach.m_LoadOp),
				.storeOp = ConversionsVK::GetVkStoreOp(depthAttach.m_StoreOp),
			};
			vkDepthAttach.clearValue.depthStencil = {1.0f, 0};
		}

		// Render Info
		VkRenderingInfo vkRenderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.flags = 0,
			.renderArea = VkRect2D{
				.offset = VkOffset2D{0, 0}, .extent = {renderingInfo.m_RenderArea.x, renderingInfo.m_RenderArea.y}
			},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = u32(vkColAttach.size()),
			.pColorAttachments = vkColAttach.data(),
			.pDepthAttachment = renderingInfo.m_UseDepthAttachment ? &vkDepthAttach : nullptr,
			.pStencilAttachment = renderingInfo.m_UseDepthAttachment ? &vkDepthAttach : nullptr,
		};
		vkCmdBeginRendering(m_CmdBuffer, &vkRenderingInfo);
	}

	void GraphicsCommandList::EndRendering() {
		vkCmdEndRendering(m_CmdBuffer);
	}

	void GraphicsCommandList::SetViewport(Vec2 offset, Vec2 size, Vec2 minMaxDepth) {
		VkViewport p{
			.x = offset.x,
			.y = offset.y,
			.width = size.x,
			.height = size.y,
			.minDepth = minMaxDepth.x,
			.maxDepth = minMaxDepth.y,
		};
		vkCmdSetViewport(m_CmdBuffer, 0, 1, &p);
	}

	void GraphicsCommandList::SetScissor(Int2 offset, UInt2 extent) {
		VkRect2D rect{
			.offset = VkOffset2D{offset.x, offset.y},
			.extent = VkExtent2D{extent.x, extent.y}
		};
		vkCmdSetScissor(m_CmdBuffer, 0, 1, &rect);
	}

	void GraphicsCommandList::SetPipeline(PipelineHandle pipeline) {
		vkCmdBindPipeline(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		                  m_pDevice->m_ResourcesDB.GetPipeline(pipeline).m_vkPipeline);
	}

	void GraphicsCommandList::SetDefaultViewportScissor(Vec2 size) {
		SetViewport({0.0f, 0.0f}, size, {0.0f, 1.0f});
		SetScissor({0, 0}, size);
	}

	void GraphicsCommandList::SetVertexBuffer(BufferHandle bufferHandle) {
		VkBuffer     vertexBuffers[] = {m_pDevice->m_ResourcesDB.GetBuffer(bufferHandle)->m_vkBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(m_CmdBuffer, 0, 1, vertexBuffers, offsets);
	}

	void GraphicsCommandList::SetIndexBuffer(BufferHandle bufferHandle, u64 offset) {
		vkCmdBindIndexBuffer(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(bufferHandle)->m_vkBuffer, offset,
		                     VK_INDEX_TYPE_UINT32);
	}

	void GraphicsCommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
		vkCmdDraw(m_CmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void GraphicsCommandList::DrawIndexed(u64 indexCount, u64 firstInstance) {
		vkCmdDrawIndexed(m_CmdBuffer, indexCount, 1, 0, 0, firstInstance);
	}

	void GraphicsCommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) {
		vkCmdDrawIndexed(m_CmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void GraphicsCommandList::BindDescriptor(PipelineHandle pipeline, DescriptorSetHandle set) {
		Pipeline&       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* layout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline.m_PipelineLayout);
		DescriptorSet&  descriptorSets = m_pDevice->m_ResourcesDB.GetDescriptorSet(set, m_pDevice->m_CurrFrameInFlightIdx);
		vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        layout->m_vkPipelineLayout,
		                        descriptorSets.m_LayoutIndex, 1,
		                        &descriptorSets.m_DescriptorSet, 0, 0);
	}

	void GraphicsCommandList::PushConstant(PipelineHandle pipeline, u64 size, void* data) {
		Pipeline&       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* pLayout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline.m_PipelineLayout);
		vkCmdPushConstants(m_CmdBuffer, pLayout->m_vkPipelineLayout,
		                   VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
	}

	void GraphicsCommandList::Barrier(TextureBarrierDescription desc) {
		Vector<TextureBarrierDescription> temp{desc};
		Barrier(temp);
	}

	void GraphicsCommandList::Barrier(Vector<TextureBarrierDescription> const& descVec) {
		Vector<VkImageMemoryBarrier2> vkBarriers{};
		vkBarriers.reserve(descVec.size());

		for (TextureBarrierDescription const& desc : descVec) {
			VkImage image = m_pDevice->m_ResourcesDB.GetTexture(desc.m_Texture)->m_vkImage;
			CKE_ASSERT(image != VK_NULL_HANDLE);
			VkImageMemoryBarrier2 vkImageBarrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = ConversionsVK::GetVkPipelineStageFlags(desc.m_SrcStage),
				.srcAccessMask = ConversionsVK::GetVkAccessFlags(desc.m_SrcAccessMask),
				.dstStageMask = ConversionsVK::GetVkPipelineStageFlags(desc.m_DstStage),
				.dstAccessMask = ConversionsVK::GetVkAccessFlags(desc.m_DstAccessMask),
				.oldLayout = ConversionsVK::GetVkImageLayout(desc.m_OldLayout),
				.newLayout = ConversionsVK::GetVkImageLayout(desc.m_NewLayout),
				.srcQueueFamilyIndex = desc.m_SrcQueueFamilyIdx,
				.dstQueueFamilyIndex = desc.m_DstQueueFamilyIdx,
				.image = image,
				.subresourceRange = {
					.aspectMask = ConversionsVK::GetVkImageAspectFlags(desc.m_AspectMask),
					.baseMipLevel = desc.m_Range.m_BaseMip,
					.levelCount = desc.m_Range.m_MipCount,
					.baseArrayLayer = desc.m_Range.m_BaseLayer,
					.layerCount = desc.m_Range.m_LayerCount,
				}
			};
			vkBarriers.emplace_back(vkImageBarrier);
		}

		VkDependencyInfo dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = nullptr,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = nullptr,
			.imageMemoryBarrierCount = static_cast<u32>(vkBarriers.size()),
			.pImageMemoryBarriers = vkBarriers.data(),
		};

		vkCmdPipelineBarrier2(m_CmdBuffer, &dependencyInfo);
	}

	void TransferCommandList::Begin() {
		vkResetCommandBuffer(m_CmdBuffer, 0);

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(m_CmdBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
			std::cout << "Error beginning to record command buffer" << std::endl;
		}
	}

	void TransferCommandList::Barrier(TextureBarrierDescription desc) {
		VkImage image = m_pDevice->m_ResourcesDB.GetTexture(desc.m_Texture)->m_vkImage;
		CKE_ASSERT(image != VK_NULL_HANDLE);

		VkImageMemoryBarrier2 imageBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = ConversionsVK::GetVkPipelineStageFlags(desc.m_SrcStage),
			.srcAccessMask = ConversionsVK::GetVkAccessFlags(desc.m_SrcAccessMask),
			.dstStageMask = ConversionsVK::GetVkPipelineStageFlags(desc.m_DstStage),
			.dstAccessMask = ConversionsVK::GetVkAccessFlags(desc.m_DstAccessMask),
			.oldLayout = ConversionsVK::GetVkImageLayout(desc.m_OldLayout),
			.newLayout = ConversionsVK::GetVkImageLayout(desc.m_NewLayout),
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = ConversionsVK::GetVkImageAspectFlags(desc.m_AspectMask),
				.baseMipLevel = desc.m_Range.m_BaseMip,
				.levelCount = desc.m_Range.m_MipCount,
				.baseArrayLayer = desc.m_Range.m_BaseLayer,
				.layerCount = desc.m_Range.m_LayerCount,
			}
		};

		VkDependencyInfo dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
			.dependencyFlags = 0,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = nullptr,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = nullptr,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageBarrier,
		};

		vkCmdPipelineBarrier2(m_CmdBuffer, &dependencyInfo);
	}

	void TransferCommandList::CopyBuffer(BufferHandle src, BufferHandle dst, u64 size) {
		VkBufferCopy copyRegion{};
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(src)->m_vkBuffer,
		                m_pDevice->m_ResourcesDB.GetBuffer(dst)->m_vkBuffer, 1, &copyRegion);
	}

	void TransferCommandList::CopyTexture(TextureCopyInfo srcInfo, TextureCopyInfo dstInfo, UInt3 size) {
		RenderResourcesDB* pDb = &m_pDevice->m_ResourcesDB;
		Texture*           srcTex = pDb->GetTexture(srcInfo.m_TexHandle);
		Texture*           dstTex = pDb->GetTexture(dstInfo.m_TexHandle);

		VkImageCopy copy{
			.srcSubresource = VkImageSubresourceLayers{
				.aspectMask = ConversionsVK::GetVkImageAspectFlags(srcInfo.m_AspectMask),
				.mipLevel = srcInfo.m_MipLevel,
				.baseArrayLayer = srcInfo.m_BaseArrayLayer,
				.layerCount = srcInfo.m_ArrayLayerCount
			},
			.srcOffset = VkOffset3D{srcInfo.m_Offset.x, srcInfo.m_Offset.y, srcInfo.m_Offset.z},
			.dstSubresource = VkImageSubresourceLayers{
				.aspectMask = ConversionsVK::GetVkImageAspectFlags(dstInfo.m_AspectMask),
				.mipLevel = dstInfo.m_MipLevel,
				.baseArrayLayer = dstInfo.m_BaseArrayLayer,
				.layerCount = dstInfo.m_ArrayLayerCount
			},
			.dstOffset = VkOffset3D{dstInfo.m_Offset.x, dstInfo.m_Offset.y, dstInfo.m_Offset.z},
			.extent = VkExtent3D{size.x, size.y, size.z}
		};
		VkImageLayout vkSrcLayout = ConversionsVK::GetVkImageLayout(srcInfo.m_Layout);
		VkImageLayout vkDstLayout = ConversionsVK::GetVkImageLayout(dstInfo.m_Layout);
		vkCmdCopyImage(m_CmdBuffer, srcTex->m_vkImage, vkSrcLayout, dstTex->m_vkImage, vkDstLayout, 1, &copy);
	}

	void TransferCommandList::CopyBufferToTexture(BufferHandle src, TextureHandle dst, VkBufferImageCopy copyRegion) {
		vkCmdCopyBufferToImage(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(src)->m_vkBuffer,
		                       m_pDevice->m_ResourcesDB.GetTexture(dst)->m_vkImage,
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                       1, &copyRegion);
	}

	void TransferCommandList::CopyTextureToBuffer(TextureHandle     src, BufferHandle dst,
	                                              VkBufferImageCopy copyRegion) {
		vkCmdCopyImageToBuffer(m_CmdBuffer,
		                       m_pDevice->m_ResourcesDB.GetTexture(src)->m_vkImage,
		                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		                       m_pDevice->m_ResourcesDB.GetBuffer(dst)->m_vkBuffer,
		                       1, &copyRegion);
	}

	void ComputeCommandList::BindComputeDescriptor(PipelineHandle pipeline, DescriptorSetHandle set) {
		Pipeline&       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* layout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline.m_PipelineLayout);
		DescriptorSet&  descriptorSets = m_pDevice->m_ResourcesDB.GetDescriptorSet(set, m_pDevice->m_CurrFrameInFlightIdx);
		vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		                        layout->m_vkPipelineLayout,
		                        descriptorSets.m_LayoutIndex, 1,
		                        &descriptorSets.m_DescriptorSet, 0, 0);
	}

	void ComputeCommandList::SetComputePipeline(PipelineHandle pipeline) {
		vkCmdBindPipeline(m_CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		                  m_pDevice->m_ResourcesDB.GetPipeline(pipeline).m_vkPipeline);
	}

	void ComputeCommandList::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) {
		vkCmdDispatch(m_CmdBuffer, groupCountX, groupCountY, groupCountZ);
	}
}
