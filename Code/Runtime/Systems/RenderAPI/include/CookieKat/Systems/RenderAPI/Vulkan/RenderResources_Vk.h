#pragma once

#include "CookieKat/Systems/RenderAPI/Internal/RenderResource.h"
#include "CookieKat/Systems/RenderAPI/RenderSettings.h"
#include "CookieKat/Systems/RenderAPI/Pipeline.h"

#include <vulkan/vulkan_core.h>

namespace CKE {
	// Shorthand for an array that contains data that has a per-frame copy
	template <typename T>
	using FrameArray = Array<T, RenderSettings::MAX_FRAMES_IN_FLIGHT>;

	class PipelineLayout : public RenderResource<PipelineLayout>
	{
	public:
		i32                             m_DescriptorSetsInUse = 0;
		Array<VkDescriptorSetLayout, 4> m_DescriptorSetLayouts;
		VkPipelineLayout                m_vkPipelineLayout;
		PipelineLayoutDesc              m_Desc;
	};

	class Pipeline : public RenderResource<Pipeline>
	{
	public:
		PipelineLayoutHandle m_PipelineLayout;
		VkPipeline           m_vkPipeline;
	};

	class Semaphore : public RenderResource<Semaphore>
	{
	public:
		VkSemaphore m_vkSemaphore;
	};

	class Fence : public RenderResource<Fence>
	{
	public:
		VkFence m_vkFence;
	};

	class Buffer : public RenderResource<Buffer>
	{
	public:
		VkBuffer       m_vkBuffer;
		VkDeviceMemory m_vkMemory;
		u64            m_Offset;
		u64            m_Size;
		bool           m_IsPerFrame;
		void*          m_pMapped = nullptr;
	};

	class Texture : public RenderResource<Texture>
	{
	public:
		Vector<TextureViewHandle> m_ExistingViews{};
		VkImage                   m_vkImage;
		VkDeviceMemory            m_vkDeviceMemory;
	};

	class TextureView : public RenderResource<TextureView>
	{
	public:
		TextureHandle m_Texture;
		VkImageView   m_vkView;
	};

	class TextureSampler : public RenderResource<TextureSampler>
	{
	public:
		VkSampler m_vkSampler;
	};


	class DescriptorSet : public RenderResource<DescriptorSet>
	{
	public:
		VkDescriptorSet m_DescriptorSet;
		u32             m_LayoutIndex;
	};
}
