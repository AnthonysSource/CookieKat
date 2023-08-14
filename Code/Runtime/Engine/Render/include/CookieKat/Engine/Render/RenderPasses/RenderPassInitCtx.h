#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Systems/RenderUtils/TextureSamplersCache.h"

#include "CookieKat/Engine/Render/PipelineManager/PipelineManager.h"

namespace CKE {
	struct RenderingSettings;
}

namespace CKE {
	class RenderPassInitCtx
	{
	public:
		RenderPassInitCtx(RenderDevice*   pDevice, TextureSamplersCache* pSamplersCache, ResourceSystem* pResources,
		                  EntityDatabase* pEntityDB, RenderingSettings*  pView, PipelineManager* pPipelineManager) :
			m_pDevice{pDevice}, m_pSamplerCache{pSamplersCache}, m_pResources{pResources}, m_pEntityDB{pEntityDB},
			m_pView{pView}, m_pPipelineManager{pPipelineManager} {}

		RenderDevice*            GetDevice() const { return m_pDevice; }
		TextureSamplersCache*    GetSamplerCache() const { return m_pSamplerCache; }
		PipelineManager*         GetPipelineManager() const { return m_pPipelineManager; }
		RenderingSettings const* GetRenderingSettings() const { return m_pView; }

		ResourceSystem* GetResourceSystem() const { return m_pResources; }
		EntityDatabase* GetEntityDatabase() const { return m_pEntityDB; }

	private:
		RenderDevice*   m_pDevice{nullptr};
		ResourceSystem* m_pResources{nullptr};
		EntityDatabase* m_pEntityDB{nullptr};

		TextureSamplersCache* m_pSamplerCache{nullptr};
		PipelineManager*      m_pPipelineManager{nullptr};
		RenderingSettings*    m_pView{nullptr};
	};
}