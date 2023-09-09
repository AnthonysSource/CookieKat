#include "RenderingSystem.h"

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Profilling/Profilling.h"

#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Systems/EngineSystem/SystemsRegistry.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

#include "CookieKat/Engine/Resources/Resources/RenderTextureResource.h"
#include "CookieKat/Engine/Entities/EntitySystem.h"
#include "CookieKat/Engine/Entities/Components/MeshComponent.h"

#include "CookieKat/Engine/Render/Common/GlobalRenderAssets.h"
#include "CookieKat/Systems/RenderUtils/SphericalHarmonicsUtils.h"

#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"
#include "CookieKat/Engine/Render/RenderPasses/BloomModule.h"
#include "CookieKat/Engine/Render/RenderPasses/DepthPrePass.h"
#include "CookieKat/Engine/Render/RenderPasses/FXAAPass.h"
#include "CookieKat/Engine/Render/RenderPasses/GBufferPass.h"
#include "CookieKat/Engine/Render/RenderPasses/LightingPass.h"
#include "CookieKat/Engine/Render/RenderPasses/PresentPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SkyBoxPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SSAOPass.h"
#include "CookieKat/Engine/Render/RenderPasses/TonemappingPass.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

//-----------------------------------------------------------------------------

namespace CKE {
	FGImportedTextureDesc GetBackBufferImportedDesc(RenderDevice* pDevice) {
		FGImportedTextureDesc desc{};
		desc.m_fgID = General::Swapchain;
		desc.m_TexHandle = pDevice->GetBackBuffer();
		desc.m_TexDesc = pDevice->GetBackBufferDesc();
		desc.m_FullTexView = pDevice->GetBackBufferView();
		return desc;
	}

	void LoadDefaultPipelines(PipelineManager* pPipelineManager) {
		// TODO: Get pipeline loading out of the render passes
		{
			GraphicsPipelineDesc desc;
			desc.m_AttachmentsInfo = AttachmentsInfo{
				{},
				TextureFormat::D24_UNORM_S8_UINT,
			};
			desc.m_BlendState = BlendState{
				{AttachmentBlendState{}}
			};
			desc.m_DepthStencilState = DepthStencilState{
				true, true, CompareOp::LessOrEqual, false
			};
			pPipelineManager->CreateFromAsset(PipelineIDS::DepthPrePass, "Shaders/depthPrePass.pipeline", desc);
		}

		{
			GraphicsPipelineDesc desc;
			desc.m_AttachmentsInfo = AttachmentsInfo{
				{
					TextureFormat::R16G16B16A16_SFLOAT,
					TextureFormat::R32G32B32A32_SFLOAT,
					TextureFormat::R16G16B16A16_SFLOAT,
					TextureFormat::R8G8B8A8_UNORM,
					TextureFormat::R16G16B16A16_SFLOAT,
				},
				TextureFormat::D24_UNORM_S8_UINT,
			};
			desc.m_BlendState = BlendState{
				{
					AttachmentBlendState{},
					AttachmentBlendState{},
					AttachmentBlendState{},
					AttachmentBlendState{},
					AttachmentBlendState{},
				}
			};
			desc.m_DepthStencilState = DepthStencilState{
				true, false, CompareOp::LessOrEqual, false
			};
			pPipelineManager->CreateFromAsset(PipelineIDS::GBufferPass, "Shaders/gPass.pipeline", desc);
		}
	}
}

//-----------------------------------------------------------------------------

namespace CKE {
	void RenderingSystem::Initialize(SystemsRegistry* pSystemsRegistry) {
		// Store references to system dependencies
		m_pResources = pSystemsRegistry->GetSystem<ResourceSystem>();
		m_pEntitySystem = pSystemsRegistry->GetSystem<EntitySystem>();
		m_pTaskSystem = pSystemsRegistry->GetSystem<TaskSystem>();

		// Init Core Rendering Systems
		m_RTexManager.Initialize(&m_Device);
		m_SamplerCache.Initialize(&m_Device);
		m_PipelineManager.Initialize(&m_Device, m_pResources);

		// Create Base Resources
		GlobalRenderAssets::Initialize(m_Device);

		LoadDefaultPipelines(&m_PipelineManager);
		m_RenderSceneManager.InitializeGPUBuffers(&m_Device);

		// TEMP: SkyBox testing
		//-----------------------------------------------------------------------------

		TextureViewHandle skyboxView;
		{
			auto cubeMapID = m_pResources->LoadResource<RenderCubeMapResource>("EnvMaps/EnvMap_Test.cubeMap");
			auto cubeMap = m_pResources->GetResource<RenderCubeMapResource>(cubeMapID);
			skyboxView = cubeMap->GetTextureView();

			Vec3 envSHCoeffs[9];
			SphericalHarmonicsUtils::SHProjectCubeMap(envSHCoeffs, cubeMap->GetFaces(), cubeMap->GetFaceSize());
			EnvironmentGPU e{};
			for (int i = 0; i < 9; ++i) {
				e.m_EnvSH.m_Coeffs[i] = Vec4{envSHCoeffs[i], 0.0f};
				for (Vec4& coeff : e.m_TestSH[i].m_Coeffs) {
					coeff = Vec4{1.0f};
				}
			}
			m_RenderSceneManager.SetEnviorementData(e);
		}

		// Setup Scene, Render Passes and FrameGraph
		//-----------------------------------------------------------------------------

		RenderPassInitCtx initCtx{
			&m_Device, &m_SamplerCache, m_pResources, m_pEntitySystem->GetEntityDatabase(),
			&m_RenderSceneManager.m_RenderingViewSettings, &m_PipelineManager
		};
		m_DepthPass = CKE::New<DepthPrePass>();
		m_DepthPass->Initialize(&initCtx);
		m_GBufferPass = CKE::New<GBufferPass>();
		m_GBufferPass->Initialize(&initCtx);
		m_LightingPass = CKE::New<LightingPass>();
		m_LightingPass->Initialize(&initCtx);
		m_SSAOPass = CKE::New<SSAOPass>();
		m_SSAOPass->Initialize(&initCtx, m_pEntitySystem->GetEntityDatabase(), m_pResources);
		m_SSAOBlurPass = CKE::New<BlurPass>();
		m_SSAOBlurPass->Initialize(&initCtx, m_pResources);
		m_TonemappingPass = CKE::New<ToneMappingPass>();
		m_TonemappingPass->Initialize(&initCtx, m_pResources);
		m_FXAAPass = CKE::New<FXAAPass>();
		m_FXAAPass->Initialize(&initCtx, m_pResources);
		m_SkyBoxPass = CKE::New<SkyBoxPass>();
		m_SkyBoxPass->Initialize(&initCtx, m_pResources, skyboxView);
		m_BloomModule = CKE::New<BloomModule>();
		m_BloomModule->Initialize(&initCtx, m_pResources);
		m_PresentPass = CKE::New<PresentPass>();
		m_PresentPass->Initialize(&m_Device, &m_SamplerCache, m_pResources);

		// Setup FrameGraph
		//-----------------------------------------------------------------------------

		m_FrameGraph.Initialize(&m_Device);
		m_FrameGraph.AddGraphicsPass(m_DepthPass);
		m_FrameGraph.AddGraphicsPass(m_GBufferPass);
		m_FrameGraph.AddGraphicsPass(m_SSAOPass);
		m_FrameGraph.AddGraphicsPass(m_SSAOBlurPass);
		m_FrameGraph.AddGraphicsPass(m_LightingPass);
		m_FrameGraph.AddGraphicsPass(m_SkyBoxPass);
		m_BloomModule->AddToGraph(&m_FrameGraph);
		m_FrameGraph.AddGraphicsPass(m_TonemappingPass);
		m_FrameGraph.AddGraphicsPass(m_FXAAPass);
		m_FrameGraph.AddGraphicsPass(m_PresentPass);

		// Import external data into FrameGraph
		//-----------------------------------------------------------------------------

		FGImportedTextureDesc swapChainDesc{};
		swapChainDesc.m_fgID = General::Swapchain;
		swapChainDesc.m_TexHandle = m_Device.GetBackBuffer();
		swapChainDesc.m_TexDesc = m_Device.GetBackBufferDesc();
		swapChainDesc.m_FullTexView = m_Device.GetBackBufferView();
		swapChainDesc.m_InitialLayout = TextureLayout::Present_Src;
		swapChainDesc.m_SrcStageWhenAvailable = PipelineStage::BottomOfPipe;
		swapChainDesc.m_SrcAccessMaskWhenAvailable = AccessMask::None;
		swapChainDesc.m_LoadOp = LoadOp::DontCare;
		m_FrameGraph.ImportTexture(swapChainDesc);
		m_FrameGraph.ImportBuffer(SceneGlobal::ObjectData, m_RenderSceneManager.m_ObjectDataBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::View, m_RenderSceneManager.m_ViewBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::Lights, m_RenderSceneManager.m_LightsBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::EnviorementData, m_RenderSceneManager.m_EnviorementBuffer);

		m_FrameGraph.Compile(m_Device.GetBackBufferSize());
	}

	void RenderingSystem::RenderFrame() {
		CKE_PROFILE_EVENT();

		m_Device.AcquireNextBackBuffer();

		// Update rendering buffers and config
		m_RenderSceneManager.CopySceneDataFromEntityWorld(&m_Device, m_pEntitySystem->GetEntityDatabase());
		m_RenderSceneManager.SetRenderingViewSettings(RenderingSettings{
			m_Device.GetBackBufferSize(),
			ViewportData{Vec2{0.0f}, m_Device.GetBackBufferSize()}
		});

		// Update BackBuffer texture reference and execute the FrameGraph
		m_FrameGraph.UpdateImportedTexture(GetBackBufferImportedDesc(&m_Device));
		m_FrameGraph.Execute(
			{m_Device.GetImageAvailableSemaphore(), PipelineStage::ColorAttachmentOutput},
			m_Device.GetRenderFinishedSemaphore(),
			m_Device.GetInFlightFence());

		m_Device.Present();

		// Handle BackBuffer Resized Trigger
		if (m_TriggerBackBufferResize) {
			m_RTexManager.UpdateRenderTarget(m_Device.GetBackBufferSize());
			m_FrameGraph.UpdateRenderTargetSize(m_Device.GetBackBufferSize());
			m_TriggerBackBufferResize = false;
		}
	}

	void RenderingSystem::Shutdown() {
		m_Device.WaitForDevice();
		m_FrameGraph.Shutdown();
		m_RenderSceneManager.CleanupGPUBuffers(&m_Device);
		GlobalRenderAssets::Shutdown(m_Device);
	}

	void RenderingSystem::RecordRenderTargetResizeEvent(Int2 newSize) {
		m_Device.RecordBackBufferResized(newSize);
		m_TriggerBackBufferResize = true;
	}

	void RenderingSystem::InitializeRenderTarget(void* pRT) {
		m_Device.SetRenderTargetData(pRT);
	}

	void RenderingSystem::InitializeDevice() {
		m_Device.Initialize(Int2(1280, 720));
	}
}
