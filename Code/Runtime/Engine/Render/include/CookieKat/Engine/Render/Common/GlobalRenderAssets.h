#pragma once

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	class EntityDatabase;

	// Simple Globally accessible rendering assets like default textures or basic meshes 
	class GlobalRenderAssets
	{
	public:
		inline static BufferHandle      GetScreenQuad() { return m_ScreenQuad; }
		inline static BufferHandle      GetCubeMapMesh() { return m_CubeMapMesh; }
		inline static TextureViewHandle White1x1() { return m_White1x1_View; }
		inline static TextureViewHandle GetBlack1x1() { return m_Black1x1_View; }
		inline static TextureViewHandle NormalDefault() { return m_NormalDefault_View; }

	private:
		friend class RenderingSystem;

		static void Initialize(RenderDevice& device);
		static void Shutdown(RenderDevice& device);

	private:
		inline static BufferHandle m_ScreenQuad;

		inline static TextureHandle     m_White1x1;
		inline static TextureViewHandle m_White1x1_View;
		inline static TextureHandle     m_Black1x1;
		inline static TextureViewHandle m_Black1x1_View;
		inline static TextureHandle     m_NormalDefault;
		inline static TextureViewHandle m_NormalDefault_View;
		inline static BufferHandle      m_CubeMapMesh;
	};
}