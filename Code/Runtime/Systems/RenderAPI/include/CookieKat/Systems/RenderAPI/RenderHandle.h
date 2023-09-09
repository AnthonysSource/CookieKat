#pragma once

#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	class RenderHandle
	{
	public:
		u64 m_Value = 0;

	public:
		RenderHandle() = default;
		RenderHandle(u64 rawHandle) : m_Value(rawHandle) { }

		// Returns an Invalid Resource Handle
		inline static RenderHandle Invalid();

		inline bool                IsValid() const;
		inline bool                IsNull() const;
		friend bool                operator==(const RenderHandle& lhs, const RenderHandle& rhs);
		friend bool                operator!=(const RenderHandle& lhs, const RenderHandle& rhs);
	};

	template <typename T>
	class TRenderHandle : public RenderHandle
	{
	public:
		// Returns an Invalid Resource Handle
		inline static TRenderHandle<T> Invalid() { return static_cast<TRenderHandle<T>>(RenderHandle::Invalid()); }
	};
}

// STD Hash declarations so that handles can be used un maps
//-----------------------------------------------------------------------------

template <>
struct std::hash<CKE::RenderHandle>
{
	std::size_t operator()(const CKE::RenderHandle& k) const noexcept {
		return k.m_Value;
	}
};

template <typename T>
struct std::hash<CKE::TRenderHandle<T>>
{
	std::size_t operator()(const CKE::TRenderHandle<T>& k) const noexcept {
		return k.m_Value;
	}
};

// Forward declared resource handles
//-----------------------------------------------------------------------------

namespace CKE {
	class Texture;
	class TextureView;
	class TextureSampler;
	class Buffer;
	class DescriptorSet;
	class Pipeline;
	class PipelineLayout;
	class Semaphore;
	class Fence;
	class CommandQueue;

	using BufferHandle = TRenderHandle<Buffer>;

	using TextureHandle = TRenderHandle<Texture>;
	using TextureViewHandle = TRenderHandle<TextureView>;
	using SamplerHandle = TRenderHandle<TextureSampler>;

	using SemaphoreHandle = TRenderHandle<Semaphore>;
	using FenceHandle = TRenderHandle<Fence>;

	using DescriptorSetHandle = TRenderHandle<DescriptorSet>;
	using PipelineHandle = TRenderHandle<Pipeline>;
	using PipelineLayoutHandle = TRenderHandle<PipelineLayout>;

	using CommandQueueHandle = TRenderHandle<CommandQueue>;

	template class TRenderHandle<Buffer>;

	template class TRenderHandle<Texture>;
	template class TRenderHandle<TextureView>;
	template class TRenderHandle<TextureSampler>;

	template class TRenderHandle<Semaphore>;
	template class TRenderHandle<Fence>;

	template class TRenderHandle<DescriptorSet>;
	template class TRenderHandle<Pipeline>;
	template class TRenderHandle<PipelineLayout>;

	template class TRenderHandle<CommandQueue>;
}

//-----------------------------------------------------------------------------

namespace CKE {
	inline bool RenderHandle::IsValid() const { return m_Value != 0; }
	inline bool RenderHandle::IsNull() const { return m_Value == 0; }

	inline bool operator==(const RenderHandle& lhs, const RenderHandle& rhs) {
		return lhs.m_Value == rhs.m_Value;
	}

	inline bool operator!=(const RenderHandle& lhs, const RenderHandle& rhs) {
		return !(lhs == rhs);
	}

	inline RenderHandle RenderHandle::Invalid() {
		return RenderHandle{0};
	}
}
