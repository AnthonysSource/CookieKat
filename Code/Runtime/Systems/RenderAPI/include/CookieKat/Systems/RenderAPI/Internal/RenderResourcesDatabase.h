#pragma once

#include "CookieKat/Core/Memory/PoolAllocator.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/FrameData.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"

namespace CKE {
	// Contains meta-data of a DB resource
	struct ResourceInfo
	{
		// We currently handle per-frame resources internally in the RenderAPI
		// which makes everything a bit complicated. This was a mistake.
		// TODO: Extract per-frame resources functionality outside of the main RenderAPI?
		bool m_PerFrame = false;
	};

	// Handles the lifetime and storage of the internal representation of render resources
	// like buffers, textures, pipelines, etc...
	class RenderResourcesDatabase
	{
	public:
		// Default constructor that initializes the database
		RenderResourcesDatabase();

		//-----------------------------------------------------------------------------

		// Called by the device when we go to a new frame
		inline void UpdateFrameIdx(u32 newIdx) { m_FrameIdx = newIdx; }

		// Buffers
		//-----------------------------------------------------------------------------

		Buffer*             CreateBuffer();
		FrameArray<Buffer*> CreateBuffersPerFrame();

		bool IsPerFrame(BufferHandle handle);
		// Returns the buffer associated with the given handle
		// If the buffer is PerFrame then it will return the current buffer for the frame
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

		// Queue
		//-----------------------------------------------------------------------------

		CommandQueue* CreateQueue();
		CommandQueue* GetQueue(CommandQueueHandle handle);
		void          RemoveQueue(CommandQueueHandle handle);

		// Pipelines
		//-----------------------------------------------------------------------------

		PipelineLayout* CreatePipelineLayout();
		PipelineLayout* GetPipelineLayout(PipelineLayoutHandle handle);
		void            RemovePipelineLayout(PipelineLayoutHandle handle);

		Pipeline*                       CreatePipeline();
		Pipeline*                       GetPipeline(PipelineHandle handle);
		void                            RemovePipeline(PipelineHandle handle);
		Map<PipelineHandle, Pipeline*>& GetAllPipelines();

		// Synchronization
		//-----------------------------------------------------------------------------

		Semaphore* AddSemaphore();
		Semaphore* GetSemaphore(SemaphoreHandle handle);

		Fence* CreateFence();
		Fence* GetFence(FenceHandle handle);

		// Descriptors
		//-----------------------------------------------------------------------------

		FrameArray<DescriptorSet*> CreateDescriptorSet();
		DescriptorSet*             CreateDescriptorSetForFrame(u32 frameIdx);
		DescriptorSet&             GetDescriptorSet(DescriptorSetHandle handle, u32 frameIdx);
		void                       DestroyAllDescriptorSets(u32 frameIdx);
		void                       CreatePipelineFrameData(PipelineHandle renderHandle);

	private:
		friend class RenderDeviceDebugUtils;

		template <typename T>
		T   GenerateResourceHandle();
		u64 m_LastResourceHandle = 0;

	private:
		template <typename K, typename V>
		class PooledMap
		{
		public:
			explicit PooledMap(u64 maxElements) : TPoolAllocator<V>{maxElements} {
				m_Map.reserve(maxElements);
			}

			~PooledMap() {
				Memory::Free(m_Allocator.GetUnderlyingMemoryBuffer());
			}


			PooledMap(const PooledMap& other) = delete;
			PooledMap(PooledMap&& other) noexcept = delete;

			V* Get(K handle) {
				return m_Map.at(handle);
			}

			void Add(K handle, V value) {
				CKE_ASSERT(!m_Map.contains(handle));
				m_Map.insert({handle, m_Allocator.New(value)});
			}

			void Delete(K handle) {
				CKE_ASSERT(m_Map.contains(handle));
				V* ptr = m_Map.at(handle);
				m_Allocator.Free(ptr);
				m_Map.erase(handle);
			}

			Map<K, V*>& GetMap() { return m_Map; }

		private:
			TPoolAllocator<V> m_Allocator{};
			Map<K, V*>        m_Map{};
		};

		u32                        m_FrameIdx = 0;
		FrameArray<FrameResources> m_FrameResources{};

		PooledMap<RenderHandle, ResourceInfo> m_ResourceInfo{5'000};

		PooledMap<PipelineHandle, Pipeline>             m_Pipelines{100};
		PooledMap<PipelineLayoutHandle, PipelineLayout> m_PipelineLayouts{100};

		PooledMap<BufferHandle, Buffer> m_Buffers{1000};

		PooledMap<TextureHandle, Texture>         m_Textures{1000};
		PooledMap<TextureViewHandle, TextureView> m_TextureViews{1000};
		PooledMap<SamplerHandle, TextureSampler>  m_TextureSamplers{50};

		PooledMap<SemaphoreHandle, Semaphore> m_Semaphores{500};
		PooledMap<FenceHandle, Fence>         m_Fences{500};

		PooledMap<CommandQueueHandle, CommandQueue> m_CommandQueues{30};
	};
}

namespace CKE {
	template <typename T>
	T RenderResourcesDatabase::GenerateResourceHandle() {
		m_LastResourceHandle++;
		return T{m_LastResourceHandle};
	}
}
