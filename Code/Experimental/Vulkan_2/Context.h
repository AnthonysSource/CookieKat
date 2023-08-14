#pragma once
#include "Auxiliary.h"
#include "RenderBackend.h"

class Buffer;
class Pipeline;

class Context
{
public:
	void Begin();
	void End();

protected:
	friend class VulkanDevice;
	VkCommandBuffer m_CommandBuffer;
};

class GraphicsContext : public Context
{
public:
	void BeginRenderPass(VkRenderPass renderPass, u32 imageIdx);
	void EndRenderPass();

	void SetViewport(VkViewport& viewport);
	void SetScissor(VkRect2D& scissor);
	void SetPipeline(Pipeline& pipeline);

	void SetVertexBuffer(Buffer buffer);
	void SetIndexBuffer(Buffer buffer, u64 offset);
	void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
	void DrawIndexed(u64 indexCount, u64 indexOffset);

	void BindDescriptor(VkPipelineLayout layout, VkDescriptorSet& set);
	void PushConstant(VkPipelineLayout layout, u64 size, void* data);

private:
	friend class VulkanDevice;
	SwapChainData* m_SwapChain = nullptr;
};

class TransferContext : public Context
{
public:
	void Begin();

	void CopyBuffer(Buffer src, Buffer dst, VkDeviceSize size);
	void CopyBufferToTexture(Buffer src, VkImage dst, VkDeviceSize size);
};