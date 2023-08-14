#include "LightProbeSystem/LightProbeBakingSystem.h"

#include "RenderPasses/DepthPrePass.h"
#include "RenderPasses/GBufferPass.h"
#include "RenderPasses/LightingPass.h"
#include "RenderPasses/SharedIDs.h"
#include "RenderPasses/SkyBoxPass.h"
#include "RenderPasses/SSAOPass.h"
#include "RenderScene/RenderSceneManager.h"

#include "stb_image_write.h"

namespace CKE {
	void LightProbeBakingSystem::Initialize(RenderDevice* pDevice,
	                                        RenderSceneManager*  pSceneData,
	                                        DepthPrePass* DepthPass,
	                                        GBufferPass*  GBufferPass,
	                                        SSAOPass*     SSAOPass,
	                                        BlurPass*     SSAOBlurPass,
	                                        LightingPass* LightingPass,
	                                        SkyBoxPass*   skyboxPass) {
		m_pDevice = pDevice;
		m_pRenderScene = pSceneData;

		BufferDesc bufferDesc{};
		bufferDesc.m_Name = "CubeMap ReadBack Buffer";
		bufferDesc.m_Usage = BufferUsage::TransferDst | BufferUsage::TransferSrc;
		bufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
		bufferDesc.m_UpdateFrequency = UpdateFrequency::Static;
		bufferDesc.m_SizeInBytes = sizeof(float) * 4 * 64 * 64;
		m_ReadBackBuffer = m_pDevice->CreateBuffer(bufferDesc);

		TextureDesc textureDesc{};
		textureDesc.m_Name = "LightProbe";
		textureDesc.m_Format = TextureFormat::R32G32B32A32_SFLOAT;
		textureDesc.m_Size = UInt3{64, 64, 1};
		textureDesc.m_AspectMask = TextureAspectMask::Color;
		textureDesc.m_MiscFlags = TextureMiscFlags::Texture_CubeMap;
		textureDesc.m_ArraySize = 6;
		textureDesc.m_Usage = TextureUsage::Color_Attachment | TextureUsage::Transfer_Dst | TextureUsage::Transfer_Src;
		m_CubeMapTex = pDevice->CreateTexture(textureDesc);
		GraphicsCommandList c = pDevice->GetGraphicsCmdList();
		c.Begin();
		c.Barrier(TextureBarrierDescription{
			PipelineStage::AllCommands,
			AccessMask::None,
			PipelineStage::Transfer,
			AccessMask::Transfer_Read,
			TextureLayout::Undefined,
			TextureLayout::Transfer_Dst,
			m_CubeMapTex,
			TextureAspectMask::Color,
			TextureRange{
				TextureAspectMask::Color,
				0, 1,
				0, 6
			}
		});
		c.End();
		pDevice->SubmitGraphicsCommandList(c, CmdListSubmitInfo{});

		m_CopyToCubemapPass.SetTargetCubeMap(m_CubeMapTex);

		m_FrameGraph.Initialize(m_pDevice);
		m_FrameGraph.AddGraphicsPass(DepthPass);
		m_FrameGraph.AddGraphicsPass(GBufferPass);
		m_FrameGraph.AddGraphicsPass(SSAOPass);
		m_FrameGraph.AddGraphicsPass(SSAOBlurPass);
		m_FrameGraph.AddGraphicsPass(LightingPass);
		m_FrameGraph.AddGraphicsPass(skyboxPass);
		m_FrameGraph.AddTransferPass(&m_CopyToCubemapPass);

		m_FrameGraph.ImportBuffer(SceneGlobal::ObjectData, pSceneData->m_ObjectDataBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::View, pSceneData->m_ViewBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::Lights, pSceneData->m_LightsBuffer);
		m_FrameGraph.ImportBuffer(SceneGlobal::EnviorementData, pSceneData->m_EnviorementBuffer);

		Vec2 renderSize{64, 64};
		pSceneData->SetRenderingViewSettings(RenderingSettings{
			renderSize,
			ViewportData{Vec2{0.0f}, renderSize}
		});
		Mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.005f, 1'000.0f);
		proj[1][1] *= -1;
		pSceneData->SetCameraMatrices(pSceneData->m_Scene.m_ViewData.m_View,
		                              proj
		);
		m_FrameGraph.Compile(renderSize);
		m_FrameGraph.UpdateRenderTargetSize(renderSize);
	}

	struct LookDir
	{
		Vec3 forward;
		Vec3 up;
	};

	SHCoeffs9 LightProbeBakingSystem::RenderProbe(Vec3 pos) {
		FenceHandle f = m_pDevice->CreateFence(false);

		LookDir dirs[6] = {
			{Vec3{1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}},
			{Vec3{-1.0, 0.0, 0.0}, Vec3{0.0, 1.0, 0.0}},
			{Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.0, 1.0}},
			{Vec3{0.0, -1.0, 0.0}, Vec3{0.0, 0.0, -1.0}},
			{Vec3{0.0, 0.0, 1.0}, Vec3{0.0, 1.0, 0.0}},
			{Vec3{0.0, 0.0, -1.0}, Vec3{0.0, 1.0, 0.0}},
		};

		for (i32 i = 0; i < 6; ++i) {
			m_CopyToCubemapPass.SetFaceToCopy(i);
			m_pRenderScene->SetCameraMatrices(
				glm::lookAt(pos, pos + dirs[i].forward, dirs[i].up)
				, m_pRenderScene->m_Scene.m_ViewData.m_Proj
			);

			m_FrameGraph.Execute({SemaphoreHandle{0}, PipelineStage::AllCommands}, SemaphoreHandle{0}, f);
			m_pDevice->WaitForFence(f);
			m_pDevice->ResetFence(f);
			m_pDevice->WaitForDevice();

			{
				GraphicsCommandList c = m_pDevice->GetGraphicsCmdList();
				c.Begin();
				c.Barrier(TextureBarrierDescription{
					PipelineStage::AllCommands,
					AccessMask::None,
					PipelineStage::Transfer,
					AccessMask::Transfer_Read,
					TextureLayout::Transfer_Dst,
					TextureLayout::Transfer_Src,
					m_CubeMapTex,
					TextureAspectMask::Color,
					TextureRange{
						TextureAspectMask::Color,
						0, 1,
						0, 6
					}
				});
				c.End();
				m_pDevice->SubmitGraphicsCommandList(c, CmdListSubmitInfo{});
				m_pDevice->WaitForDevice();
			}

			{
				VkBufferImageCopy c{};
				c.bufferOffset = sizeof(float) * 4 * 64 * 64 * 0;
				c.bufferImageHeight = 0;
				c.bufferRowLength = 0;
				c.imageExtent = VkExtent3D{64, 64, 1};
				c.imageOffset = VkOffset3D{0, 0, 0};
				c.imageSubresource = VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, static_cast<u32>(i), 1};
				TransferCommandList t = m_pDevice->GetTransferCmdList();
				t.Begin();
				t.CopyTextureToBuffer(m_CubeMapTex, m_ReadBackBuffer, c);
				t.End();
				m_pDevice->SubmitTransferCommandList(t, CmdListSubmitInfo{});
				m_pDevice->WaitForDevice();
			}

			{
				GraphicsCommandList c = m_pDevice->GetGraphicsCmdList();
				c.Begin();
				c.Barrier(TextureBarrierDescription{
					PipelineStage::AllCommands,
					AccessMask::None,
					PipelineStage::Transfer,
					AccessMask::Transfer_Read,
					TextureLayout::Transfer_Src,
					TextureLayout::Transfer_Dst,
					m_CubeMapTex,
					TextureAspectMask::Color,
					TextureRange{
						TextureAspectMask::Color,
						0, 1,
						0, 6
					}
				});
				c.End();
				m_pDevice->SubmitGraphicsCommandList(c, CmdListSubmitInfo{});
				m_pDevice->WaitForDevice();
			}

			void* p = m_pDevice->GetBufferMappedPtr_DEPR(m_ReadBackBuffer);

			String name{"Face_"};
			name = name.append(std::to_string(i));
			name = name.append(".hdr");
			stbi_write_hdr(name.c_str(), 64, 64, 4, (f32*)p);
		}

		return {};
	}

	void LightProbeBakingSystem::Shutdown() { }
}

template <typename T>
T BillinearInterp(Vec2 v, T v00, T v01, T v10, T v11) {
	T r1 = v00 * (1.0f - v.x) + v10 * v.x;
	T r2 = v01 * (1.0f - v.x) + v11 * v.x;
	T r3 = r1 * (1.0f - v.y) + r2 * v.y;
	return r3;
}

namespace CKE {
	LightProbeGrid LightProbeBakingSystem::GenerateProbeGrid() {
		LightProbeGrid grid{};
		for (int x = -9; x < 10; ++x) {
			for (int z = -9; z < 10; ++z) { }
		}
		return LightProbeGrid{};
	}

	void LightProbeGrid::Setup(Mat4 gridToWorld) {
		m_WorldToLightGrid = glm::identity<Mat4>();//glm::inverse(gridToWorld);
		for (i32 x = 0; x < 20; ++x) {
			m_Probes.push_back(Vector<Vector<LightProbeData>>{});
			for (i32 y = 0; y < 20; ++y) {
				m_Probes[x].push_back(Vector<LightProbeData>{});
				for (i32 z = 0; z < 20; ++z) {
					m_Probes[x][y].push_back(LightProbeData{});
				}
			}
		}
	}

	SHCoeffs9 LightProbeGrid::GetInterpolatedSHCoeffsAtPosition(Vec3 worldPos) {
		Vec4 worldPos4 = Vec4{worldPos, 1.0f};
		Vec3 localPos = m_WorldToLightGrid * worldPos4;
		Int3 p = Int3{glm::floor(localPos)};
		Int3 p000 = p;
		Int3 p100 = Int3{p.x + 1, p.y, p.z};
		Int3 p010 = Int3{p.x, p.y + 1, p.z};
		Int3 p110 = Int3{p.x + 1, p.y + 1, p.z};

		Int3 p001 = Int3{p.x, p.y, p.z + 1};
		Int3 p101 = Int3{p.x + 1, p.y, p.z + 1};
		Int3 p011 = Int3{p.x, p.y + 1, p.z + 1};
		Int3 p111 = Int3{p.x + 1, p.y + 1, p.z + 1};

		SHCoeffs9 interpCoeffs9{};
		for (i32 i = 0; i < 9; ++i) {
			Vec3 boxCoords = localPos - Vec3{Int3{localPos}};

			Vec3 sh000 = GetProbe(p000)->m_Coeffs.m_Value[i];
			Vec3 sh010 = GetProbe(p010)->m_Coeffs.m_Value[i];
			Vec3 sh100 = GetProbe(p100)->m_Coeffs.m_Value[i];
			Vec3 sh110 = GetProbe(p110)->m_Coeffs.m_Value[i];

			Vec3 sh001 = GetProbe(p001)->m_Coeffs.m_Value[i];
			Vec3 sh011 = GetProbe(p011)->m_Coeffs.m_Value[i];
			Vec3 sh101 = GetProbe(p101)->m_Coeffs.m_Value[i];
			Vec3 sh111 = GetProbe(p111)->m_Coeffs.m_Value[i];

			Vec3 r1 = BillinearInterp(boxCoords, sh000, sh010, sh100, sh110);
			Vec3 r2 = BillinearInterp(boxCoords, sh001, sh011, sh101, sh111);
			Vec3 r3 = r1 * (1.0f - boxCoords.z) + r2 * boxCoords.z;
			interpCoeffs9.m_Value[i] = r3;
		}
		return interpCoeffs9;
	}

	void LightProbeGrid::SetProbe(Int3 localPos, SHCoeffs9 coeffs) {
		LightProbeData probeData{localPos, coeffs};
		m_Probes[localPos.x][localPos.y][localPos.z] = probeData;
	}

	LightProbeData* LightProbeGrid::GetProbe(Int3 localPos) {
		return &m_Probes[localPos.x][localPos.y][localPos.z];
	}
}

namespace CKE {
	void CopyToCubeMapPass::SetFaceToCopy(i32 faceIdx) {
		m_CurrentFaceIdx = faceIdx;
	}

	void CopyToCubeMapPass::SetTargetCubeMap(TextureHandle cubeMap) {
		m_TargetCubeMap = cubeMap;
	}

	void CopyToCubeMapPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(LightingPass::HDRSceneColor, FGPipelineAccessInfo{
			                 PipelineStage::Transfer,
			                 AccessMask::Transfer_Read,
			                 TextureLayout::Transfer_Src,
			                 TextureAspectMask::Color,
			                 LoadOp::Load
		                 });
	}

	void CopyToCubeMapPass::Execute(ExecuteResourcesCtx& ctx, TransferCommandList& cmdList, RenderDevice& rd) {
		TextureHandle sceneColor = ctx.GetTexture(LightingPass::HDRSceneColor);

		TextureCopyInfo srcInfo{};
		srcInfo.m_AspectMask = TextureAspectMask::Color;
		srcInfo.m_Layout = TextureLayout::Transfer_Src;
		srcInfo.m_TexHandle = sceneColor;

		TextureCopyInfo dstInfo{};
		dstInfo.m_AspectMask = TextureAspectMask::Color;
		dstInfo.m_Layout = TextureLayout::Transfer_Dst;
		dstInfo.m_TexHandle = m_TargetCubeMap;
		dstInfo.m_BaseArrayLayer = m_CurrentFaceIdx;

		cmdList.CopyTexture(srcInfo, dstInfo, UInt3(64, 64, 1));
	}
}
