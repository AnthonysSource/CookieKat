#include "RenderScene/RenderSceneManager.h"

#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

#include "CookieKat/Engine/Entities/Components/CameraComponent.h"
#include "CookieKat/Engine/Entities/Components/LocalToWorldComponent.h"
#include "CookieKat/Engine/Entities/Components/MeshComponent.h"
#include "CookieKat/Engine/Entities/Components/PointLightComponent.h"

namespace CKE {
	void RenderSceneManager::InitializeGPUBuffers(RenderDevice* pDevice) {
		m_pDevice = pDevice;
		m_Scene.m_ObjectData.resize(RenderSettings::MAX_OBJECTS);

		BufferDesc objectDataBufferDesc{};
		objectDataBufferDesc.m_Name = "Object Data Buffer";
		objectDataBufferDesc.m_Usage = BufferUsage::Storage | BufferUsage::TransferDst;
		objectDataBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		objectDataBufferDesc.m_UpdateFrequency = UpdateFrequency::PerFrame;
		objectDataBufferDesc.m_SizeInBytes = sizeof(ObjectDataGPU) * RenderSettings::MAX_OBJECTS;
		objectDataBufferDesc.m_StrideInBytes = sizeof(ObjectDataGPU);
		m_ObjectDataBuffer = pDevice->CreateBuffer(objectDataBufferDesc);

		BufferDesc viewDataBufferDesc{};
		viewDataBufferDesc.m_Name = "View Buffer";
		viewDataBufferDesc.m_Usage = BufferUsage::Uniform | BufferUsage::TransferDst;
		viewDataBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		viewDataBufferDesc.m_UpdateFrequency = UpdateFrequency::PerFrame;
		viewDataBufferDesc.m_SizeInBytes = sizeof(ViewDataGPU);
		m_ViewBuffer = pDevice->CreateBuffer(viewDataBufferDesc);

		BufferDesc lightDataBufferDesc{};
		lightDataBufferDesc.m_Name = "Light Data Buffer";
		lightDataBufferDesc.m_Usage = BufferUsage::Uniform | BufferUsage::TransferDst;
		lightDataBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		lightDataBufferDesc.m_UpdateFrequency = UpdateFrequency::PerFrame;
		lightDataBufferDesc.m_SizeInBytes = sizeof(LightsDataGPU);
		m_LightsBuffer = pDevice->CreateBuffer(lightDataBufferDesc);

		BufferDesc enviorementBufferDesc{};
		enviorementBufferDesc.m_Name = "Enviorement Buffer";
		enviorementBufferDesc.m_Usage = BufferUsage::Uniform | BufferUsage::TransferDst;
		enviorementBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		enviorementBufferDesc.m_UpdateFrequency = UpdateFrequency::PerFrame;
		enviorementBufferDesc.m_SizeInBytes = sizeof(EnvironmentGPU);
		m_EnviorementBuffer = m_pDevice->CreateBuffer(enviorementBufferDesc);
	}

	void RenderSceneManager::SetCameraMatrices(Mat4 view, Mat4 proj) {
		m_Scene.m_ViewData.m_View = view;
		m_Scene.m_ViewData.m_Proj = proj;
		m_Scene.m_ViewData.m_ProjInv = glm::inverse(proj);
		m_Scene.m_ViewData.m_ViewInv = glm::inverse(view);
		m_Scene.m_ViewData.m_ViewProj = proj * view;
		m_pDevice->UploadBufferData_DEPR(m_ViewBuffer, &m_Scene.m_ViewData, sizeof(ViewDataGPU), 0);
	}

	void RenderSceneManager::SetRenderingViewSettings(RenderingSettings view) {
		m_RenderingViewSettings = view;
	}

	void RenderSceneManager::SetEnviorementData(EnvironmentGPU data) {
		m_Scene.m_EnviorementData = data;
		m_pDevice->UploadBufferData_DEPR(m_EnviorementBuffer, &m_Scene.m_EnviorementData,
		                                 sizeof(EnvironmentGPU), 0);
	}

	void RenderSceneManager::CopySceneDataFromEntityWorld(RenderDevice*   pDevice,
	                                                      EntityDatabase* pEntities) {
		// Upload object data of all of the objects in the scene
		//-----------------------------------------------------------------------------

		int objCount = 0;
		for (auto& [l2w, mesh] : pEntities->GetMultiCompTupleIter<
			     LocalToWorldComponent, MeshComponent>()) {
			ObjectDataGPU obj{};
			obj.m_Local2World = l2w->m_LocalToWorld;
			obj.m_NormalMat = glm::transpose(glm::inverse(l2w->m_LocalToWorld));
			obj.m_AlbedoOverride = mesh->m_MaterialModifiers.m_Albedo;
			obj.m_MetallicOverride = mesh->m_MaterialModifiers.m_MetalMask;
			obj.m_RoughnessOverride = mesh->m_MaterialModifiers.m_Roughness;
			obj.m_Reflectance = mesh->m_MaterialModifiers.m_Reflectance;
			CKE_ASSERT(mesh->m_ObjectIdx - 1 >= 0 && mesh->m_ObjectIdx < RenderSettings::MAX_OBJECTS);
			m_Scene.m_ObjectData[mesh->m_ObjectIdx - 1] = obj;
			objCount++;
		}
		pDevice->UploadBufferData_DEPR(m_ObjectDataBuffer, m_Scene.m_ObjectData.data(),
		                               objCount * sizeof(ObjectDataGPU), 0);

		// Init Main Camera
		//-----------------------------------------------------------------------------

		for (auto& [cam] : pEntities->GetMultiCompTupleIter<CameraComponent>()) {
			Mat4 projFlipped = cam->m_Proj;
			Mat4 view = cam->m_View;

			projFlipped[1][1] *= -1;
			m_Scene.m_ViewData.m_View = view;
			m_Scene.m_ViewData.m_Proj = projFlipped;
			m_Scene.m_ViewData.m_ProjInv = glm::inverse(projFlipped);
			m_Scene.m_ViewData.m_ViewInv = glm::inverse(view);
			m_Scene.m_ViewData.m_ViewProj = projFlipped * view;

			pDevice->UploadBufferData_DEPR(m_ViewBuffer, &m_Scene.m_ViewData, sizeof(ViewDataGPU), 0);
		}

		// Collect and upload scene lights
		//-----------------------------------------------------------------------------

		m_Scene.m_LightsData.m_Size.x = 0;
		int idx = 0;
		for (auto pointLight : pEntities->GetSingleCompIter<PointLightComponent>()) {
			m_Scene.m_LightsData.m_PointLights[idx].m_ViewSpacePosition = m_Scene.m_ViewData.m_View
					* Vec4(
						pointLight->m_Position, 1.0f);
			m_Scene.m_LightsData.m_PointLights[idx].m_Radiance = Vec4(pointLight->m_Radiance, 0.0f);
			m_Scene.m_LightsData.m_Size.x++;
			idx++;
		}
		pDevice->UploadBufferData_DEPR(m_LightsBuffer, &m_Scene.m_LightsData,
		                               sizeof(Vec4) + sizeof(PointLightGPU) * idx, 0);
	}

	void RenderSceneManager::CleanupGPUBuffers(RenderDevice* pDevice) {
		pDevice->DestroyBuffer(m_LightsBuffer);
		pDevice->DestroyBuffer(m_ObjectDataBuffer);
		pDevice->DestroyBuffer(m_ViewBuffer);
		pDevice->DestroyBuffer(m_EnviorementBuffer);
	}
} // namespace CKE
