#include "CookieKat/Systems/RenderAPI/Internal/RenderResourcesDatabase.h"

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"

namespace CKE {
	RenderResourcesDatabase::RenderResourcesDatabase() {
		for (u64 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			m_FrameResources[i].m_DescriptorSets.reserve(RenderSettings::MAX_OBJECTS);
		}
	}

	Buffer* RenderResourcesDatabase::CreateBuffer() {
		TRenderHandle<Buffer> handle = GenerateResourceHandle<BufferHandle>();
		ResourceInfo          info{};
		info.m_PerFrame = false;
		m_ResourceInfo.Add(RenderHandle{handle.m_Value}, info);

		Buffer buffer{};
		buffer.m_DBHandle = handle;
		m_Buffers.Add(handle, buffer);
		return m_Buffers.Get(handle);
	}

	FrameArray<Buffer*> RenderResourcesDatabase::CreateBuffersPerFrame() {
		TRenderHandle<Buffer> handle = GenerateResourceHandle<BufferHandle>();
		ResourceInfo          info{};
		info.m_PerFrame = true;
		m_ResourceInfo.Add(RenderHandle{handle.m_Value}, info);

		FrameArray<Buffer*> buffers{};

		for (i32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			Buffer buffer{};
			buffer.m_DBHandle = handle;
			m_FrameResources[i].m_Buffers.insert({handle, buffer});
			Buffer* pBuffer = &m_FrameResources[i].m_Buffers[handle];
			buffers[i] = pBuffer;
		}

		return buffers;
	}

	bool RenderResourcesDatabase::IsPerFrame(BufferHandle handle) {
		return m_ResourceInfo.Get(handle)->m_PerFrame;
	}

	Buffer* RenderResourcesDatabase::GetBuffer(BufferHandle handle) {
		if (m_ResourceInfo.Get(handle)->m_PerFrame == true) {
			return &m_FrameResources[m_FrameIdx].m_Buffers[handle];
		}
		else {
			return m_Buffers.Get(handle);
		}
	}

	FrameArray<Buffer*> RenderResourcesDatabase::GetBuffer(BufferHandle handle, bool& isPerFrame) {
		isPerFrame = m_ResourceInfo.Get(handle)->m_PerFrame;
		FrameArray<Buffer*> b{};

		if (m_ResourceInfo.Get(handle)->m_PerFrame == true) {
			for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				b[i] = &m_FrameResources[i].m_Buffers[handle];
			}
		}
		else {
			for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				b[i] = m_Buffers.Get(handle);
			}
		}

		return b;
	}

	void RenderResourcesDatabase::RemoveBuffer(BufferHandle handle) {
		if (m_ResourceInfo.Get(handle)->m_PerFrame == true) {
			for (int i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
				m_FrameResources[i].m_Buffers.erase(handle);
			}
		}
		else {
			m_Buffers.Delete(handle);
		}

		m_ResourceInfo.Delete(handle);
	}

	Texture* RenderResourcesDatabase::CreateTexture() {
		Texture texture{};
		texture.m_DBHandle = GenerateResourceHandle<TextureHandle>();
		m_Textures.Add(texture.m_DBHandle, texture);
		return m_Textures.Get(texture.m_DBHandle);
	}

	Texture* RenderResourcesDatabase::GetTexture(TextureHandle handle) {
		return m_Textures.Get(handle);
	}

	void RenderResourcesDatabase::RemoveTexture(TextureHandle handle) {
		m_Textures.Delete(handle);
	}

	TextureView* RenderResourcesDatabase::CreateTextureView() {
		TextureView textureView{};
		textureView.m_DBHandle = GenerateResourceHandle<TextureViewHandle>();
		m_TextureViews.Add(textureView.m_DBHandle, textureView);
		return m_TextureViews.Get(textureView.m_DBHandle);
	}

	TextureView* RenderResourcesDatabase::GetTextureView(TextureViewHandle handle) {
		return m_TextureViews.Get(handle);
	}

	void RenderResourcesDatabase::RemoveTextureView(TextureViewHandle handle) {
		m_TextureViews.Delete(handle);
	}

	TextureSampler* RenderResourcesDatabase::CreateTextureSampler() {
		TextureSampler sampler{};
		sampler.m_DBHandle = GenerateResourceHandle<SamplerHandle>();
		m_TextureSamplers.Add(sampler.m_DBHandle, sampler);
		return m_TextureSamplers.Get(sampler.m_DBHandle);
	}

	TextureSampler* RenderResourcesDatabase::GetTextureSampler(SamplerHandle handle) {
		return m_TextureSamplers.Get(handle);
	}

	void RenderResourcesDatabase::RemoveTextureSampler(SamplerHandle handle) {
		m_TextureSamplers.Delete(handle);
	}

	CommandQueue* RenderResourcesDatabase::CreateQueue() {
		CommandQueue sampler{};
		sampler.m_DBHandle = GenerateResourceHandle<CommandQueueHandle>();
		m_CommandQueues.Add(sampler.m_DBHandle, sampler);
		return m_CommandQueues.Get(sampler.m_DBHandle);
	}

	CommandQueue* RenderResourcesDatabase::GetQueue(CommandQueueHandle handle) {
		return m_CommandQueues.Get(handle);
	}

	void RenderResourcesDatabase::RemoveQueue(CommandQueueHandle handle) {
		m_CommandQueues.Delete(handle);
	}

	PipelineLayout* RenderResourcesDatabase::CreatePipelineLayout() {
		PipelineLayout layout{};
		layout.m_DBHandle = GenerateResourceHandle<PipelineLayoutHandle>();
		m_PipelineLayouts.Add(layout.m_DBHandle, layout);
		return m_PipelineLayouts.Get(layout.m_DBHandle);
	}

	PipelineLayout* RenderResourcesDatabase::GetPipelineLayout(PipelineLayoutHandle handle) {
		return m_PipelineLayouts.Get(handle);
	}

	void RenderResourcesDatabase::RemovePipelineLayout(PipelineLayoutHandle handle) {
		m_PipelineLayouts.Delete(handle);
	}

	Pipeline* RenderResourcesDatabase::CreatePipeline() {
		auto handle = GenerateResourceHandle<PipelineHandle>();
		m_Pipelines.Add(handle, Pipeline{handle});
		return m_Pipelines.Get(handle);
	}

	Pipeline* RenderResourcesDatabase::GetPipeline(PipelineHandle handle) {
		return m_Pipelines.Get(handle);
	}

	void RenderResourcesDatabase::RemovePipeline(PipelineHandle handle) {
		m_Pipelines.Delete(handle);
	}

	Semaphore* RenderResourcesDatabase::AddSemaphore() {
		auto handle = GenerateResourceHandle<SemaphoreHandle>();
		m_Semaphores.Add(handle, Semaphore{handle});
		return m_Semaphores.Get(handle);
	}

	Semaphore* RenderResourcesDatabase::GetSemaphore(SemaphoreHandle handle) {
		return m_Semaphores.Get(handle);
	}

	Fence* RenderResourcesDatabase::CreateFence() {
		auto handle = GenerateResourceHandle<FenceHandle>();
		m_Fences.Add(handle, Fence{handle});
		return m_Fences.Get(handle);
	}

	Fence* RenderResourcesDatabase::GetFence(FenceHandle handle) {
		return m_Fences.Get(handle);
	}

	FrameArray<DescriptorSet*> RenderResourcesDatabase::CreateDescriptorSet() {
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

	DescriptorSet* RenderResourcesDatabase::CreateDescriptorSetForFrame(u32 frameIdx) {
		auto          handle = GenerateResourceHandle<DescriptorSetHandle>();
		DescriptorSet s{};
		s.m_DBHandle = handle;
		m_FrameResources[frameIdx].m_DescriptorSets.insert({handle, s});
		DescriptorSet* pSet = &m_FrameResources[frameIdx].m_DescriptorSets[handle];
		return pSet;
	}

	DescriptorSet& RenderResourcesDatabase::GetDescriptorSet(DescriptorSetHandle handle, u32 frameIdx) {
		return m_FrameResources[frameIdx].m_DescriptorSets[handle];
	}

	void RenderResourcesDatabase::DestroyAllDescriptorSets(u32 frameIdx) {
		m_FrameResources[frameIdx].m_DescriptorSets.clear();
	}

	void RenderResourcesDatabase::CreatePipelineFrameData(PipelineHandle renderHandle) {
		m_FrameResources[m_FrameIdx].m_PipelineFrameData.insert({ renderHandle, {} });
	}

	Map<PipelineHandle, Pipeline*>& RenderResourcesDatabase::GetAllPipelines() {
		return m_Pipelines.GetMap();
	}
}
