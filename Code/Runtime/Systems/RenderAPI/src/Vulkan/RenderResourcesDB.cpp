#include "CookieKat/Systems/RenderAPI/Internal/RenderResourcesDB.h"

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"

namespace CKE {
	RenderResourcesDB::RenderResourcesDB() {
		for (u64 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			m_FrameResources[i].m_DescriptorSets.reserve(RenderSettings::MAX_OBJECTS);
		}
	}

	Buffer* RenderResourcesDB::CreateBuffer() {
		TRenderHandle<Buffer> handle = GenerateResourceHandle<BufferHandle>();
		ResourceInfo          info{};
		info.m_PerFrame = false;
		m_ResourceInfo.insert({RenderHandle{handle.m_Value}, info});

		Buffer buffer{};
		buffer.m_DBHandle = handle;
		m_Buffers.insert({handle, buffer});
		return &m_Buffers[handle];
	}

	FrameArray<Buffer*> RenderResourcesDB::CreateBuffersPerFrame() {
		TRenderHandle<Buffer> handle = GenerateResourceHandle<BufferHandle>();
		ResourceInfo          info{};
		info.m_PerFrame = true;
		m_ResourceInfo.insert({RenderHandle{handle.m_Value}, info});

		FrameArray<Buffer*> buffers{};

		for (int i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			Buffer buffer{};
			buffer.m_DBHandle = handle;
			m_FrameResources[i].m_Buffers.insert({handle, buffer});
			Buffer* pBuffer = &m_FrameResources[i].m_Buffers[handle];
			buffers[i] = pBuffer;
		}

		return buffers;
	}

	Buffer* RenderResourcesDB::GetBuffer(BufferHandle handle) {
		CKE_ASSERT(m_ResourceInfo.contains(handle));

		if (m_ResourceInfo[handle].m_PerFrame == true) {
			return &m_FrameResources[m_FrameIdx].m_Buffers[handle];
		}
		else {
			return &m_Buffers[handle];
		}
	}

	FrameArray<Buffer*> RenderResourcesDB::GetBuffer(BufferHandle handle, bool& isPerFrame) {
		CKE_ASSERT(m_ResourceInfo.contains(handle));

		isPerFrame = m_ResourceInfo[handle].m_PerFrame;
		FrameArray<Buffer*> b{};

		if (m_ResourceInfo[handle].m_PerFrame == true) {
			for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				b[i] = &m_FrameResources[i].m_Buffers[handle];
			}
		}
		else {
			for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				b[i] = &m_Buffers[handle];
			}
		}

		return b;
	}

	void RenderResourcesDB::RemoveBuffer(BufferHandle handle) {
		CKE_ASSERT(m_ResourceInfo.contains(handle));

		if (m_ResourceInfo[handle].m_PerFrame == true) {
			for (int i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				m_FrameResources[i].m_Buffers.erase(handle);
			}
		}
		else {
			m_Buffers.erase(handle);
		}

		m_ResourceInfo.erase(handle);
	}

	Texture* RenderResourcesDB::CreateTexture() {
		Texture texture{};
		texture.m_DBHandle = GenerateResourceHandle<TextureHandle>();
		m_Textures.insert({texture.m_DBHandle, texture});
		return &m_Textures[texture.m_DBHandle];
	}

	Texture* RenderResourcesDB::GetTexture(TextureHandle handle) {
		CKE_ASSERT(m_Textures.contains(handle));
		return &m_Textures[handle];
	}

	void RenderResourcesDB::RemoveTexture(TextureHandle handle) {
		m_Textures.erase(handle);
	}

	TextureView* RenderResourcesDB::CreateTextureView() {
		TextureView textureView{};
		textureView.m_DBHandle = GenerateResourceHandle<TextureViewHandle>();
		m_TextureViews.insert({textureView.m_DBHandle, textureView});
		return &m_TextureViews[textureView.m_DBHandle];
	}

	TextureView* RenderResourcesDB::GetTextureView(TextureViewHandle handle) {
		CKE_ASSERT(m_TextureViews.contains(handle));
		return &m_TextureViews[handle];
	}

	void RenderResourcesDB::RemoveTextureView(TextureViewHandle handle) {
		m_TextureViews.erase(handle);
	}

	TextureSampler* RenderResourcesDB::CreateTextureSampler() {
		TextureSampler sampler{};
		sampler.m_DBHandle = GenerateResourceHandle<SamplerHandle>();
		m_TextureSamplers.insert({sampler.m_DBHandle, sampler});
		return &m_TextureSamplers[sampler.m_DBHandle];
	}

	TextureSampler* RenderResourcesDB::GetTextureSampler(SamplerHandle handle) {
		CKE_ASSERT(m_TextureSamplers.contains(handle));
		return &m_TextureSamplers[handle];
	}

	void RenderResourcesDB::RemoveTextureSampler(SamplerHandle handle) {
		m_TextureSamplers.erase(handle);
	}

	PipelineLayout* RenderResourcesDB::CreatePipelineLayout() {
		PipelineLayout layout{};
		layout.m_DBHandle = GenerateResourceHandle<PipelineLayoutHandle>();
		m_PipelineLayouts.insert({layout.m_DBHandle, layout});
		return &m_PipelineLayouts[layout.m_DBHandle];
	}

	PipelineLayout* RenderResourcesDB::GetPipelineLayout(PipelineLayoutHandle handle) {
		CKE_ASSERT(m_PipelineLayouts.contains(handle));
		return &m_PipelineLayouts[handle];
	}

	void RenderResourcesDB::RemovePipelineLayout(PipelineLayoutHandle handle) {
		CKE_ASSERT(m_PipelineLayouts.contains(handle));
		m_PipelineLayouts.erase(handle);
	}

	Pipeline& RenderResourcesDB::CreatePipeline() {
		auto handle = GenerateResourceHandle<PipelineHandle>();
		m_Pipelines.insert({handle, Pipeline{handle}});
		return m_Pipelines[handle];
	}

	Pipeline& RenderResourcesDB::GetPipeline(PipelineHandle handle) {
		CKE_ASSERT(m_Pipelines.contains(handle));
		return m_Pipelines[handle];
	}

	void RenderResourcesDB::RemovePipeline(PipelineHandle handle) {
		m_Pipelines.erase(handle);
	}

	Semaphore& RenderResourcesDB::AddSemaphore() {
		auto handle = GenerateResourceHandle<SemaphoreHandle>();
		m_Semaphores.insert({handle, Semaphore{handle}});
		return m_Semaphores[handle];
	}

	Semaphore& RenderResourcesDB::GetSemaphore(SemaphoreHandle handle) {
		CKE_ASSERT(m_Semaphores.contains(handle));
		return m_Semaphores[handle];
	}

	Fence* RenderResourcesDB::CreateFence() {
		auto handle = GenerateResourceHandle<FenceHandle>();
		m_Fences.insert({handle, Fence{handle}});
		return &m_Fences[handle];
	}

	Fence* RenderResourcesDB::GetFence(FenceHandle handle) {
		CKE_ASSERT(m_Fences.contains(handle));
		return &m_Fences[handle];
	}

	FrameArray<DescriptorSet*> RenderResourcesDB::CreateDescriptorSet() {
		auto                       handle = GenerateResourceHandle<DescriptorSetHandle>();
		FrameArray<DescriptorSet*> sets{};

		for (u64 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			DescriptorSet s{};
			s.m_DBHandle = handle;
			m_FrameResources[i].m_DescriptorSets.insert({handle, s});
			DescriptorSet* pSet = &m_FrameResources[i].m_DescriptorSets[handle];
			sets[i] = pSet;
		}

		return sets;
	}

	DescriptorSet* RenderResourcesDB::CreateDescriptorSetForFrame(u32 frameIdx) {
		auto          handle = GenerateResourceHandle<DescriptorSetHandle>();
		DescriptorSet s{};
		s.m_DBHandle = handle;
		m_FrameResources[frameIdx].m_DescriptorSets.insert({handle, s});
		DescriptorSet* pSet = &m_FrameResources[frameIdx].m_DescriptorSets[handle];
		return pSet;
	}

	DescriptorSet& RenderResourcesDB::GetDescriptorSet(DescriptorSetHandle handle, u32 frameIdx) {
		return m_FrameResources[frameIdx].m_DescriptorSets[handle];
	}

	void RenderResourcesDB::DestroyAllDescriptorSets(u32 frameIdx) {
		m_FrameResources[frameIdx].m_DescriptorSets.clear();
	}

	Map<PipelineHandle, Pipeline>& RenderResourcesDB::GetAllPipelines() {
		return m_Pipelines;
	}
}
