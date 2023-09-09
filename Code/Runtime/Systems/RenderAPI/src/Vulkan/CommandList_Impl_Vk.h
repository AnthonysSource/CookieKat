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

		VK_CHECK_CALL(vkBeginCommandBuffer(m_CmdBuffer, &commandBufferBeginInfo));
	}

	void CommandList::End() {
		VK_CHECK_CALL(vkEndCommandBuffer(m_CmdBuffer));
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

	inline CommandList::CommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer) { }

	void CommandList::BeginRendering(RenderingInfo renderingInfo) {
		Vector<VkRenderingAttachmentInfo> vkColAttach{};

		// Color
		for (RenderingAttachment& attach : renderingInfo.m_ColorAttachments) {
			vkColAttach.push_back(VkRenderingAttachmentInfo{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				.imageView = m_pDevice->m_ResourcesDB.GetTextureView(attach.m_TextureView)->m_vkView,
				.imageLayout = ConversionsVk::GetVkImageLayout(attach.m_Layout),
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.loadOp = ConversionsVk::GetVkLoadOp(attach.m_LoadOp),
				.storeOp = ConversionsVk::GetVkStoreOp(attach.m_StoreOp),
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
				.imageLayout = ConversionsVk::GetVkImageLayout(depthAttach.m_Layout),
				.resolveMode = VK_RESOLVE_MODE_NONE,
				.loadOp = ConversionsVk::GetVkLoadOp(depthAttach.m_LoadOp),
				.storeOp = ConversionsVk::GetVkStoreOp(depthAttach.m_StoreOp),
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

	void CommandList::EndRendering() {
		vkCmdEndRendering(m_CmdBuffer);
	}

	void CommandList::SetViewport(Vec2 offset, Vec2 size, Vec2 minMaxDepth) {
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

	void CommandList::SetScissor(Int2 offset, UInt2 extent) {
		VkRect2D rect{
			.offset = VkOffset2D{offset.x, offset.y},
			.extent = VkExtent2D{extent.x, extent.y}
		};
		vkCmdSetScissor(m_CmdBuffer, 0, 1, &rect);
	}

	void CommandList::SetGraphicsPipeline(PipelineHandle pipeline) {
		SetPipeline(pipeline, PipelineBindPoint::Graphics);
	}

	void CommandList::SetPipeline(PipelineHandle pipeline, PipelineBindPoint bindPoint) {
		VkPipelineBindPoint vkPipelineBindPoint = ConversionsVk::GetVkPipelineBindPoint(bindPoint);
		vkCmdBindPipeline(m_CmdBuffer, vkPipelineBindPoint,
			m_pDevice->m_ResourcesDB.GetPipeline(pipeline)->m_vkPipeline);
	}

	void CommandList::SetDefaultViewportScissor(Vec2 size) {
		SetViewport({0.0f, 0.0f}, size, {0.0f, 1.0f});
		SetScissor({0, 0}, size);
	}

	void CommandList::SetVertexBuffer(BufferHandle bufferHandle) {
		SetVertexBuffer(bufferHandle, 0, 1, 0);
	}

	void CommandList::SetVertexBuffer(BufferHandle bufferHandle, u32 firstBinding, u32 bindingCount, u64 offset) {
		VkBuffer* pVkBuffer = &m_pDevice->m_ResourcesDB.GetBuffer(bufferHandle)->m_vkBuffer;
		vkCmdBindVertexBuffers(m_CmdBuffer, firstBinding, bindingCount, pVkBuffer, &offset);
	}

	void CommandList::SetIndexBuffer(BufferHandle bufferHandle, u64 offset) {
		SetIndexBuffer(bufferHandle, offset, IndicesFormat::UINT32);
	}

	void CommandList::SetIndexBuffer(BufferHandle bufferHandle, u64 offset, IndicesFormat indicesFormat) {
		VkIndexType indexType = ConversionsVk::GetVkIndexFormat(indicesFormat);
		vkCmdBindIndexBuffer(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(bufferHandle)->m_vkBuffer, offset, indexType);
	}

	void CommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance) {
		vkCmdDraw(m_CmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::DrawIndexed(u64 indexCount, u64 firstInstance) {
		vkCmdDrawIndexed(m_CmdBuffer, indexCount, 1, 0, 0, firstInstance);
	}

	void CommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance) {
		vkCmdDrawIndexed(m_CmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandList::BindDescriptor(PipelineHandle pipeline, DescriptorSetHandle descriptorSet) {
		Pipeline*       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* layout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline->m_PipelineLayout);
		DescriptorSet&  descriptorSets = m_pDevice->m_ResourcesDB.GetDescriptorSet(descriptorSet, m_pDevice->m_CurrFrameInFlightIdx);
		vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        layout->m_vkPipelineLayout,
		                        descriptorSets.m_LayoutIndex, 1,
		                        &descriptorSets.m_DescriptorSet, 0, 0);
	}

	void CommandList::PushConstant(PipelineHandle pipeline, u64 dataSize, void* data) {
		Pipeline*       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* pLayout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline->m_PipelineLayout);
		vkCmdPushConstants(m_CmdBuffer, pLayout->m_vkPipelineLayout,
		                   VK_SHADER_STAGE_VERTEX_BIT, 0, dataSize, data);
	}

	void CommandList::Barrier(TextureBarrierDescription desc) {
		Barrier(&desc, 1);
	}

	void CommandList::Barrier(TextureBarrierDescription const* pDesc, u32 count) {
		Vector<VkImageMemoryBarrier2> vkBarriers{};
		vkBarriers.reserve(count);

		for (u32 i = 0; i < count; ++i) {
			TextureBarrierDescription desc = pDesc[i];
			VkImage                   image = m_pDevice->m_ResourcesDB.GetTexture(desc.m_Texture)->m_vkImage;
			VkImageMemoryBarrier2     vkImageBarrier{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
				.srcStageMask = ConversionsVk::GetVkPipelineStageFlags(desc.m_SrcStage),
				.srcAccessMask = ConversionsVk::GetVkAccessFlags(desc.m_SrcAccessMask),
				.dstStageMask = ConversionsVk::GetVkPipelineStageFlags(desc.m_DstStage),
				.dstAccessMask = ConversionsVk::GetVkAccessFlags(desc.m_DstAccessMask),
				.oldLayout = ConversionsVk::GetVkImageLayout(desc.m_OldLayout),
				.newLayout = ConversionsVk::GetVkImageLayout(desc.m_NewLayout),
				.srcQueueFamilyIndex = desc.m_SrcQueueFamilyIdx,
				.dstQueueFamilyIndex = desc.m_DstQueueFamilyIdx,
				.image = image,
				.subresourceRange = {
					.aspectMask = ConversionsVk::GetVkImageAspectFlags(desc.m_AspectMask),
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

	void CommandList::EndDebugLabel() {
		pfnCmdEndDebugUtilsLabelEXT(m_CmdBuffer);
	}

	void CommandList::CopyBuffer(BufferHandle src, BufferHandle dst, BufferCopyInfo copyInfo) {
		VkBufferCopy copyRegion{};
		copyRegion.dstOffset = copyInfo.dstOffset;
		copyRegion.srcOffset = copyInfo.srcOffset;
		copyRegion.size = copyInfo.size;
		vkCmdCopyBuffer(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(src)->m_vkBuffer,
		                m_pDevice->m_ResourcesDB.GetBuffer(dst)->m_vkBuffer, 1, &copyRegion);
	}

	void CommandList::CopyTexture(TextureCopyInfo srcInfo, TextureCopyInfo dstInfo, UInt3 size) {
		RenderResourcesDatabase* pDb = &m_pDevice->m_ResourcesDB;
		Texture*                 srcTex = pDb->GetTexture(srcInfo.m_TexHandle);
		Texture*                 dstTex = pDb->GetTexture(dstInfo.m_TexHandle);

		VkImageCopy copy{
			.srcSubresource = VkImageSubresourceLayers{
				.aspectMask = ConversionsVk::GetVkImageAspectFlags(srcInfo.m_Subresource.m_AspectMask),
				.mipLevel = srcInfo.m_Subresource.m_MipLevel,
				.baseArrayLayer = srcInfo.m_Subresource.m_ArrayBaseLayer,
				.layerCount = srcInfo.m_Subresource.m_ArrayLayerCount
			},
			.srcOffset = VkOffset3D{srcInfo.m_Offset.x, srcInfo.m_Offset.y, srcInfo.m_Offset.z},
			.dstSubresource = VkImageSubresourceLayers{
				.aspectMask = ConversionsVk::GetVkImageAspectFlags(dstInfo.m_Subresource.m_AspectMask),
				.mipLevel = dstInfo.m_Subresource.m_MipLevel,
				.baseArrayLayer = dstInfo.m_Subresource.m_ArrayBaseLayer,
				.layerCount = dstInfo.m_Subresource.m_ArrayLayerCount
			},
			.dstOffset = VkOffset3D{dstInfo.m_Offset.x, dstInfo.m_Offset.y, dstInfo.m_Offset.z},
			.extent = VkExtent3D{size.x, size.y, size.z}
		};
		VkImageLayout vkSrcLayout = ConversionsVk::GetVkImageLayout(srcInfo.m_Layout);
		VkImageLayout vkDstLayout = ConversionsVk::GetVkImageLayout(dstInfo.m_Layout);
		vkCmdCopyImage(m_CmdBuffer, srcTex->m_vkImage, vkSrcLayout, dstTex->m_vkImage, vkDstLayout, 1, &copy);
	}

	VkBufferImageCopy ConvertBufferImageCopyInfo(BufferImageCopyInfo const& copy) {
		VkBufferImageCopy copyInfo{};
		copyInfo.bufferImageHeight = copy.bufferImageHeight;
		copyInfo.bufferOffset = copy.bufferOffset;
		copyInfo.bufferRowLength = copyInfo.bufferRowLength;
		copyInfo.imageExtent = VkExtent3D{copy.imageExtent.x, copy.imageExtent.y, copy.imageExtent.z};
		copyInfo.imageOffset = VkOffset3D{copy.imageOffset.x, copy.imageOffset.y, copy.imageOffset.z};
		copyInfo.imageSubresource.aspectMask = ConversionsVk::GetVkImageAspectFlags(copy.imageSubresource.m_AspectMask);
		copyInfo.imageSubresource.baseArrayLayer = copy.imageSubresource.m_ArrayBaseLayer;
		copyInfo.imageSubresource.layerCount = copy.imageSubresource.m_ArrayLayerCount;
		copyInfo.imageSubresource.mipLevel = copy.imageSubresource.m_MipLevel;
		return copyInfo;
	}

	void CommandList::CopyBufferToTexture(BufferHandle src, TextureHandle dst, BufferImageCopyInfo copyInfo) {
		VkBufferImageCopy vkCopyInfo = ConvertBufferImageCopyInfo(copyInfo);

		vkCmdCopyBufferToImage(m_CmdBuffer, m_pDevice->m_ResourcesDB.GetBuffer(src)->m_vkBuffer,
		                       m_pDevice->m_ResourcesDB.GetTexture(dst)->m_vkImage,
		                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                       1, &vkCopyInfo);
	}

	void CommandList::CopyTextureToBuffer(TextureHandle src, BufferHandle dst, BufferImageCopyInfo copyInfo) {
		VkBufferImageCopy vkCopyInfo = ConvertBufferImageCopyInfo(copyInfo);

		vkCmdCopyImageToBuffer(m_CmdBuffer,
		                       m_pDevice->m_ResourcesDB.GetTexture(src)->m_vkImage,
		                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		                       m_pDevice->m_ResourcesDB.GetBuffer(dst)->m_vkBuffer,
		                       1, &vkCopyInfo);
	}

	void CommandList::BindComputeDescriptor(PipelineHandle pipeline, DescriptorSetHandle set) {
		Pipeline*       pPipeline = m_pDevice->m_ResourcesDB.GetPipeline(pipeline);
		PipelineLayout* layout = m_pDevice->m_ResourcesDB.GetPipelineLayout(pPipeline->m_PipelineLayout);
		DescriptorSet&  descriptorSets = m_pDevice->m_ResourcesDB.GetDescriptorSet(set, m_pDevice->m_CurrFrameInFlightIdx);
		vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		                        layout->m_vkPipelineLayout,
		                        descriptorSets.m_LayoutIndex, 1,
		                        &descriptorSets.m_DescriptorSet, 0, 0);
	}

	void CommandList::SetComputePipeline(PipelineHandle pipeline) {
		vkCmdBindPipeline(m_CmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
		                  m_pDevice->m_ResourcesDB.GetPipeline(pipeline)->m_vkPipeline);
	}

	void CommandList::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ) {
		vkCmdDispatch(m_CmdBuffer, groupCountX, groupCountY, groupCountZ);
	}
}
