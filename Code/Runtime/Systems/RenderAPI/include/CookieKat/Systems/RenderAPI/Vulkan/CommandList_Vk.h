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
		CommandList() = default;

		CommandList(RenderDevice* pRenderDevice, VkCommandBuffer cmdBuffer);

		void Begin(); // Being the recording of the command list
		void End();   // End the recording of the command list

		// Dynamic Rendering
		void BeginRendering(RenderingInfo renderingInfo);
		void EndRendering();

		// Vertex & Index Buffers
		void SetVertexBuffer(BufferHandle bufferHandle, u32 firstBinding, u32 bindingCount, u64 offset);
		void SetVertexBuffer(BufferHandle bufferHandle);
		void SetIndexBuffer(BufferHandle bufferHandle, u64 offset, IndicesFormat indicesFormat);
		void SetIndexBuffer(BufferHandle bufferHandle, u64 offset);

		// Draw
		void Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);
		void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance);
		void DrawIndexed(u64 indexCount, u64 firstInstance);

		// Pipeline
		void SetPipeline(PipelineHandle pipeline, PipelineBindPoint bindPoint);
		void SetGraphicsPipeline(PipelineHandle pipeline);
		void SetViewport(Vec2 offset, Vec2 size, Vec2 minMaxDepth);
		void SetScissor(Int2 offset, UInt2 extent);
		void SetDefaultViewportScissor(Vec2 size);

		// Shader Bindings
		void BindDescriptor(PipelineHandle pipeline, DescriptorSetHandle descriptorSet);
		void PushConstant(PipelineHandle pipeline, u64 dataSize, void* data);

		// Sync & Transitions
		void Barrier(TextureBarrierDescription const* desc, u32 count);
		void Barrier(TextureBarrierDescription desc);

		// Copy
		void CopyBuffer(BufferHandle src, BufferHandle dst, BufferCopyInfo copyInfo);
		void CopyTexture(TextureCopyInfo srcInfo, TextureCopyInfo dstInfo, UInt3 size);
		void CopyBufferToTexture(BufferHandle src, TextureHandle dst, BufferImageCopyInfo copyInfo);
		void CopyTextureToBuffer(TextureHandle src, BufferHandle dst, BufferImageCopyInfo copyInfo);

		// Compute
		void BindComputeDescriptor(PipelineHandle pipeline, DescriptorSetHandle set);
		void SetComputePipeline(PipelineHandle pipeline);
		void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ);

		// Debugging
		void BeginDebugLabel(const char* pName, Vec3 color);
		void EndDebugLabel();

	private:
		RenderDevice*   m_pDevice = nullptr;
		VkCommandBuffer m_CmdBuffer{};
	};
}
