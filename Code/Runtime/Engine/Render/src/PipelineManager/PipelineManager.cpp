#include "PipelineManager/PipelineManager.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	void PipelineManager::Initialize(RenderDevice* pDevice, ResourceSystem* pResources) {
		CKE_ASSERT(pResources != nullptr);
		CKE_ASSERT(pDevice != nullptr);
		m_pResources = pResources;
		m_pDevice = pDevice;
	}

	void PipelineManager::CreateFromAsset(PipelineID idToAssign, Path assetPath, GraphicsPipelineDesc desc) {
		if (m_Cache.contains(idToAssign)) {
			CKE_UNREACHABLE_CODE();
			return;
		}

		TResourceID<PipelineResource> resourceID = m_pResources->LoadResource<PipelineResource>(assetPath);
		PipelineResource*             pipelineRes = m_pResources->GetResource<PipelineResource>(resourceID);

		desc.m_FragmentShaderSource = pipelineRes->GetFragSource();
		desc.m_VertexShaderSource = pipelineRes->GetVertSource();
		desc.m_LayoutHandle = pipelineRes->GetPipelineLayout();
		desc.m_VertexInput = pipelineRes->GetVertexInputLayoutDesc();

		CachedPipelineInfo info{};
		info.m_ID = idToAssign;
		info.m_Path = assetPath;
		info.m_Desc = desc;
		info.m_Handle = m_pDevice->CreateGraphicsPipeline(desc);
		m_Cache.insert({idToAssign, info});

		m_pResources->UnloadResource(resourceID);
	}

	PipelineHandle PipelineManager::GetPipeline(PipelineID id) {
		CKE_ASSERT(m_Cache.contains(id));
		return m_Cache[id].m_Handle;
	}
}
