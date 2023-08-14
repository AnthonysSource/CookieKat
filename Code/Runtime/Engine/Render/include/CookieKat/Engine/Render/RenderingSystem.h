#pragma once

#include "CookieKat/Core/Math/Math.h"

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

#include "CookieKat/Engine/Render/RTextureManager/RTextureManager.h"
#include "CookieKat/Engine/Render/PipelineManager/PipelineManager.h"
#include "CookieKat/Engine/Render/RenderScene/RenderSceneManager.h"
#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"
#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"

//-----------------------------------------------------------------------------

namespace CKE {
	// Forward declarations
	class SystemsRegistry;
	class EntitySystem;
	class TaskSystem;

	class DepthPrePass;
	class GBufferPass;
	class SSAOPass;
	class BlurPass;
	class LightingPass;
	class BloomModule;
	class SkyBoxPass;
	class ToneMappingPass;
	class FXAAPass;
	class PresentPass;
}

//-----------------------------------------------------------------------------

namespace CKE {
	// Primary rendering system of the engine
	class RenderingSystem : public IEngineSystem
	{
	public:
		// Used to pass the win32 window pointer
		// NOTE: Must be called before Initialize()
		void InitializeRenderTarget(void* pRT);
		void InitializeDevice();
		void Initialize(SystemsRegistry* pSystemsRegistry);
		void RenderFrame();
		void Shutdown();

		void RecordRenderTargetResizeEvent(Int2 newSize);

		inline RenderDevice& GetRenderDevice();

	private:
		EntitySystem*   m_pEntitySystem = nullptr;
		ResourceSystem* m_pResources = nullptr;
		TaskSystem*     m_pTaskSystem = nullptr;

		RenderDevice         m_Device{};
		TextureSamplersCache m_SamplerCache{};
		PipelineManager      m_PipelineManager{};
		RTextureManager      m_RTexManager{};
		FrameGraph           m_FrameGraph{};
		bool                 m_TriggerBackBufferResize = false;

		RenderSceneManager m_RenderSceneManager{};

		DepthPrePass* m_DepthPass{};
		GBufferPass* m_GBufferPass{};
		SSAOPass* m_SSAOPass{};
		BlurPass* m_SSAOBlurPass{};
		LightingPass* m_LightingPass{};
		BloomModule* m_BloomModule{};
		SkyBoxPass* m_SkyBoxPass{};
		ToneMappingPass* m_TonemappingPass{};
		FXAAPass* m_FXAAPass{};
		PresentPass* m_PresentPass{};
	};
} // namespace CKE

//-----------------------------------------------------------------------------

namespace CKE {
	RenderDevice& RenderingSystem::GetRenderDevice() { return m_Device; }
}
