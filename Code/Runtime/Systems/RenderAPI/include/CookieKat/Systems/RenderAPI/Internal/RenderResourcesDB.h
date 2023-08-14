#pragma once

#include "CookieKat/Systems/RenderAPI/Vulkan/FrameData_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"

namespace CKE {
	// Contains meta-data of a DB resource
	struct ResourceInfo
	{
		// We currently handle per-frame resources internally in the RenderAPI
		// which makes everything a bit complicated.
		// This was a mistake.
		// TODO: Extract per-frame resources functionality outside of the main RenderAPI?
		bool m_PerFrame = false;
	};

	// Handles the lifetime and storage of the internal representation of render resources
	// like buffers, textures, pipelines, etc...
	class RenderResourcesDB
	{
	public:
		// Default constructor that initializes the database
		RenderResourcesDB();

		// Called by the device when we go to a new frame
		inline void UpdateFrameIdx(u32 newIdx) { m_FrameIdx = newIdx; }

		// Buffers
		//-----------------------------------------------------------------------------

		Buffer*             CreateBuffer();
		FrameArray<Buffer*> CreateBuffersPerFrame();

		// Returns the buffer associated with the given handle
		// If the buffer is PerFrame then it will return the current
		// frame buffer
		Buffer*             GetBuffer(BufferHandle handle);
		FrameArray<Buffer*> GetBuffer(BufferHandle handle, bool& isPerFrame);
		void                RemoveBuffer(BufferHandle handle);

		// Textures
		//-----------------------------------------------------------------------------

		Texture* CreateTexture();
		Texture* GetTexture(TextureHandle handle);
		void     RemoveTexture(TextureHandle handle);

		TextureView* CreateTextureView();
		TextureView* GetTextureView(TextureViewHandle handle);
		void         RemoveTextureView(TextureViewHandle handle);

		TextureSampler* CreateTextureSampler();
		TextureSampler* GetTextureSampler(SamplerHandle handle);
		void            RemoveTextureSampler(SamplerHandle handle);

		// Pipelines
		//-----------------------------------------------------------------------------

		PipelineLayout* CreatePipelineLayout();
		PipelineLayout* GetPipelineLayout(PipelineLayoutHandle handle);
		void            RemovePipelineLayout(PipelineLayoutHandle handle);

		Pipeline&                      CreatePipeline();
		Pipeline&                      GetPipeline(PipelineHandle handle);
		void                           RemovePipeline(PipelineHandle handle);
		Map<PipelineHandle, Pipeline>& GetAllPipelines();

		// Synchronization
		//-----------------------------------------------------------------------------

		Semaphore& AddSemaphore();
		Semaphore& GetSemaphore(SemaphoreHandle handle);

		Fence* CreateFence();
		Fence* GetFence(FenceHandle handle);

		// Descriptors
		//-----------------------------------------------------------------------------

		FrameArray<DescriptorSet*> CreateDescriptorSet();
		DescriptorSet*             CreateDescriptorSetForFrame(u32 frameIdx);
		DescriptorSet&             GetDescriptorSet(DescriptorSetHandle handle, u32 frameIdx);
		void                       DestroyAllDescriptorSets(u32 frameIdx);

	private:
		friend class RenderDeviceDebugUtils;

		template <typename T>
		T   GenerateResourceHandle();
		u64 m_LastResourceHandle = 0;

	private:
		u32                        m_FrameIdx = 0;
		FrameArray<FrameResources> m_FrameResources{};

		Map<RenderHandle, ResourceInfo> m_ResourceInfo{};

		Map<BufferHandle, Buffer>     m_Buffers{};
		Map<PipelineHandle, Pipeline> m_Pipelines{};

		Map<TextureHandle, Texture>         m_Textures{};
		Map<TextureViewHandle, TextureView> m_TextureViews{};
		Map<SamplerHandle, TextureSampler>  m_TextureSamplers{};

		Map<PipelineLayoutHandle, PipelineLayout> m_PipelineLayouts{};

		Map<SemaphoreHandle, Semaphore> m_Semaphores{};
		Map<FenceHandle, Fence>         m_Fences{};
	};
}

namespace CKE {
	template <typename T>
	T RenderResourcesDB::GenerateResourceHandle() {
		m_LastResourceHandle++;
		return T{m_LastResourceHandle};
	}
}
