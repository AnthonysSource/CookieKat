#include "Context.h"

#include <iostream>

#include "VulkanDevice.h"

void Context::Begin()
{
	vkResetCommandBuffer(m_CommandBuffer, 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "Error beginning to record command buffer" << std::endl;
	}
}

void Context::End()
{
	if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
	{
		std::cout << "Error ending command buffer" << std::endl;
	}
}

void GraphicsContext::BeginRenderPass(VkRenderPass renderPass, u32 imageIdx)
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.framebuffer = m_SwapChain->m_FrameBuffers[imageIdx];

	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_SwapChain->m_Extent;

	VkClearValue clearColor = { {{1.0f, 0.7f, 0.0f, 1.0f}} };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	// Commands

	vkCmdBeginRenderPass(m_CommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GraphicsContext::EndRenderPass()
{
	vkCmdEndRenderPass(m_CommandBuffer);
}

void GraphicsContext::SetViewport(VkViewport& viewport)
{
	vkCmdSetViewport(m_CommandBuffer, 0, 1, &viewport);
}

void GraphicsContext::SetScissor(VkRect2D& scissor)
{
	vkCmdSetScissor(m_CommandBuffer, 0, 1, &scissor);
}

void GraphicsContext::SetPipeline(Pipeline& pipeline)
{
	vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.m_Pipeline);
}

void GraphicsContext::SetVertexBuffer(Buffer buffer)
{
	VkBuffer vertexBuffers[] = { buffer.m_Buffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, vertexBuffers, offsets);
}

void GraphicsContext::SetIndexBuffer(Buffer buffer, u64 offset)
{
	vkCmdBindIndexBuffer(m_CommandBuffer, buffer.m_Buffer, offset, VK_INDEX_TYPE_UINT32);
}

void GraphicsContext::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void GraphicsContext::DrawIndexed(u64 indexCount, u64 indexOffset)
{
	vkCmdDrawIndexed(m_CommandBuffer, indexCount, 1, indexOffset, 0, 0);
}

void GraphicsContext::BindDescriptor(VkPipelineLayout layout, VkDescriptorSet& set)
{
	vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set, 0, 0);
}

void GraphicsContext::PushConstant(VkPipelineLayout layout, u64 size, void* data)
{
	vkCmdPushConstants(m_CommandBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
}

void TransferContext::Begin()
{
	vkResetCommandBuffer(m_CommandBuffer, 0);

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "Error beginning to record command buffer" << std::endl;
	}
}

void TransferContext::CopyBuffer(Buffer src, Buffer dst, VkDeviceSize size)
{
	VkBufferCopy copyRegion{};
	copyRegion.dstOffset = 0;
	copyRegion.srcOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(m_CommandBuffer, src.m_Buffer, dst.m_Buffer, 1, &copyRegion);
}

void TransferContext::CopyBufferToTexture(Buffer src, VkImage dst, VkDeviceSize size)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
}
