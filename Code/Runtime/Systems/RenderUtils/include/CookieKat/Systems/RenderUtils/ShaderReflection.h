#pragma once
#include "CookieKat/Systems/RenderAPI/Pipeline.h"

namespace CKE {
	// Set of utilities to extract reflected information from SPIRV shader sources
	class ShaderReflectionUtils
	{
	public:
		// Returns the required pipeline layout for a given shader
		static PipelineLayoutDesc ReflectLayout(Vector<u8> const& vertSource, Vector<u8> const& fragSource);

		// Returns the required vertex input for a vertex shader
		static VertexInputLayoutDesc ReflectVertexInput(Vector<u8> const& vertSource);
	};
}
