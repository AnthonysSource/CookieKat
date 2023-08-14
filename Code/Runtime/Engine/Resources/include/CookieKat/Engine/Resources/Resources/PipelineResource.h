#pragma once

#include "CookieKat/Systems/Resources/IResource.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class PipelineResource : public IResource
	{
		CKE_SERIALIZE(m_VertShaderSource, m_FragShaderSource);

		friend class PipelineLoader;
		friend class ResourceCompiler;

	public:
		inline Blob const&               GetFragSource() const { return m_FragShaderSource; }
		inline Blob const&               GetVertSource() const { return m_VertShaderSource; }
		inline PipelineLayoutHandle      GetPipelineLayout() const { return m_PipelineLayout; }
		inline PipelineLayoutDesc const& GetPipelineLayoutDesc() const { return m_PipelineLayoutDesc; }
		inline VertexInputLayoutDesc const& GetVertexInputLayoutDesc() const { return m_VertexInputLayoutDesc; }

	private:
		Blob                 m_VertShaderSource;
		Blob                 m_FragShaderSource;
		PipelineLayoutHandle m_PipelineLayout;
		PipelineLayoutDesc   m_PipelineLayoutDesc;
		VertexInputLayoutDesc m_VertexInputLayoutDesc;
	};
}
