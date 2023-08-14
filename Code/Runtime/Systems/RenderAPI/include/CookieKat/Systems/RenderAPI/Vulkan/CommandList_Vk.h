#pragma once

#include "CookieKat/Systems/RenderAPI/Pipeline.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"

#include <vulkan/vulkan_core.h>

namespace CKE {
	// Forward Declarations
	class SwapChain;
	class RenderDevice;
}

namespace CKE {
	class CommandList
	{
	public:
		friend RenderDevice;

		CommandList() = default;

		CommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer) :
			m_pDevice(pRenderDevice), m_CmdBuffer(cmdBuffer) {}

		// Being the recording of the command list
		void Begin();
		// End the recording of the command list
		void End();

		void BeginDebugLabel(const char* pName, Vec3 color);
		void EndDebugLabel();

	protected:
		RenderDevice*   m_pDevice = nullptr;
		VkCommandBuffer m_CmdBuffer{};
	};

	class GraphicsCommandList : public CommandList
	{
	public:
		GraphicsCommandList() = default;

		GraphicsCommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer):
			CommandList(pRenderDevice, cmdBuffer) {}

		// Dynamic Rendering
		void BeginRendering(RenderingInfo renderingInfo);
		void EndRendering();

		// Vertex Input
		void SetVertexBuffer(BufferHandle bufferHandle);
		void SetIndexBuffer(BufferHandle bufferHandle, u64 offset);

		// Drawing
		void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		void DrawIndexed(u64 indexCount, u64 firstInstance);
		void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance);

		// Pipeline
		void SetPipeline(PipelineHandle pipeline);
		void SetViewport(Vec2 offset, Vec2 size, Vec2 minMaxDepth);
		void SetScissor(Int2 offset, UInt2 extent);
		void SetDefaultViewportScissor(Vec2 size);

		// Resource Bindings
		void BindDescriptor(PipelineHandle pipeline, DescriptorSetHandle set);
		void PushConstant(PipelineHandle pipeline, u64 size, void* data);

		// Sync & Transitions
		void Barrier(TextureBarrierDescription desc);
		void Barrier(Vector<TextureBarrierDescription> const& desc);

	private:
		friend class RenderDevice;
	};

	class TransferCommandList : public CommandList
	{
	public:
		TransferCommandList() = default;

		TransferCommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer) :
			CommandList(pRenderDevice, cmdBuffer) {}

		void Begin();

		void Barrier(TextureBarrierDescription desc);
		void CopyBuffer(BufferHandle src, BufferHandle dst, u64 size);
		void CopyTexture(TextureCopyInfo srcInfo, TextureCopyInfo dstInfo, UInt3 size);
		void CopyBufferToTexture(BufferHandle src, TextureHandle dst, VkBufferImageCopy copyRegion);
		void CopyTextureToBuffer(TextureHandle src, BufferHandle dst, VkBufferImageCopy copyRegion);
	};

	class ComputeCommandList : public CommandList
	{
	public:
		ComputeCommandList() = default;

		ComputeCommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer) :
			CommandList(pRenderDevice, cmdBuffer) {}

		void BindComputeDescriptor(PipelineHandle pipeline, DescriptorSetHandle set);
		void SetComputePipeline(PipelineHandle pipeline);
		void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ);

	private:
		friend class RenderDevice;
	};
}
