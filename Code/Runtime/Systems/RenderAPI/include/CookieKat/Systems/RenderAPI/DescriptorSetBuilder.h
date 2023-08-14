#pragma once

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/Pipeline.h"

namespace CKE {
	// A DescriptorSetBuilder is created using RenderDevice::CreateDescriptorSetBuilder(...)
	class DescriptorSetBuilder
	{
	public:
		// WARNING: Do not call in user code
		DescriptorSetBuilder();

		// Binds a buffer to the specified slot as an uniform buffer
		DescriptorSetBuilder& BindUniformBuffer(u32 slot, BufferHandle buffer);

		// Binds a buffer to the specified slot as an storage buffer
		DescriptorSetBuilder& BindStorageBuffer(u32 slot, BufferHandle buffer);

		// Binds a combined texture sampler pair to the given slot
		DescriptorSetBuilder& BindTextureWithSampler(u32 slot, TextureViewHandle textureView, SamplerHandle sampler);

		// Builds a descriptor set using the binded resources
		DescriptorSetHandle Build();

	public:
		// Contains a generic descriptor binding
		struct Bindings
		{
			ShaderBindingType m_Type;
			u32               m_Slot;
			// The data inside the resourceIDs below depends
			// on the shader binding type.
			u64 m_ResourceID1;
			u64 m_ResourceID2;
		};

	private:
		friend class RenderDevice;

		RenderDevice* m_pDevice = nullptr;

		PipelineHandle   m_PipelineHandle{};
		u64              m_SetIndex{}; // Index at which the set will be bound to
		Vector<Bindings> m_Bindings{};
	};
}
