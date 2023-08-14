#pragma once

#include "CommonShapes.h"
#include "RenderSandboxEnv.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

namespace CKE {
	class RenderAPI_ComputeVertex : public IRenderSandboxTest
	{
	public:
		PipelineLayoutHandle m_ComputeLayout;
		PipelineHandle       m_GraphicsPipeline;
		PipelineHandle       m_ComputePipeline;
		BufferHandle         m_TriangleBuffer;
		SemaphoreHandle      m_ComputeFinishedSemaphore;
		PipelineLayoutHandle m_GfxLayout;

		void Setup(RenderDevice* pDevice) override {
			BufferDesc bufferDesc{};
			bufferDesc.m_Name = "Triangle Buffer";
			bufferDesc.m_MemoryAccess = MemoryAccess::GPU;
			bufferDesc.m_SizeInBytes = sizeof(CommonShapes::m_TriangleVerts);
			bufferDesc.m_StrideInBytes = sizeof(TriangleVert);
			bufferDesc.m_Usage = BufferUsage::Vertex | BufferUsage::Storage | BufferUsage::TransferDst;
			bufferDesc.m_UpdateFrequency = UpdateFrequency::Static;
			m_TriangleBuffer = pDevice->CreateBuffer_DEPR(bufferDesc, CommonShapes::m_TriangleVerts.data(),
			                                              sizeof(CommonShapes::m_TriangleVerts));

			PipelineLayoutDesc compLayoutDesc{};
			compLayoutDesc.SetShaderBindings({
				ShaderBinding{0, 0, ShaderBindingType::StorageBuffer, 1, ShaderStageMask::Compute}
			});
			m_ComputeLayout = pDevice->CreatePipelineLayout(compLayoutDesc);

			ComputePipelineDesc computeDesc{};
			computeDesc.m_Layout = m_ComputeLayout;
			computeDesc.m_ComputeShaderSrc = g_FileSystem.ReadBinaryFile("compute_particles_comp.spv");
			m_ComputePipeline = pDevice->CreateComputePipeline(computeDesc);

			m_ComputeFinishedSemaphore = pDevice->CreateSemaphoreGPU();

			PipelineLayoutDesc emptyLayout{};
			m_GfxLayout = pDevice->CreatePipelineLayout(emptyLayout);

			GraphicsPipelineDesc gfxDesc{};
			gfxDesc.m_BlendState.m_AttachmentBlendStates = {AttachmentBlendState{}};
			gfxDesc.m_AttachmentsInfo.m_ColorAttachments = {TextureFormat::B8G8R8A8_SRGB};
			gfxDesc.m_DepthStencilState.m_DepthWrite = false;
			gfxDesc.m_DepthStencilState.m_DepthTestEnable = false;
			gfxDesc.m_DepthStencilState.m_StencilEnable = false;
			gfxDesc.m_Topology = PrimitiveTopology::TriangleList;
			gfxDesc.m_VertexShaderSource = g_FileSystem.ReadBinaryFile("test_01_vert.spv");
			gfxDesc.m_FragmentShaderSource = g_FileSystem.ReadBinaryFile("test_01_frag.spv");
			gfxDesc.m_VertexInput.SetVertexInput({
				VertexInputInfo{VertexInputFormat::Float_R32G32B32, 0},
				VertexInputInfo{VertexInputFormat::Float_R32G32B32, 0},
			});
			gfxDesc.m_LayoutHandle = m_GfxLayout;
			m_GraphicsPipeline = pDevice->CreateGraphicsPipeline(gfxDesc);
		}

		void Render(RenderDevice* pDevice) override {
			pDevice->AcquireNextBackBuffer();

			ComputeCommandList compList = pDevice->GetComputeCmdList();
			compList.Begin();

			DescriptorSetBuilder builder = pDevice->CreateDescriptorSetBuilder(m_ComputePipeline, 0);
			DescriptorSetHandle  setHandle = builder.BindStorageBuffer(0, m_TriangleBuffer).Build();
			compList.BindComputeDescriptor(m_ComputePipeline, setHandle);
			compList.SetComputePipeline(m_ComputePipeline);
			compList.Dispatch(3, 1, 1);

			compList.End();

			CmdListSubmitInfo        computeSubmitInfo{};
			CmdListWaitSemaphoreInfo waitInfo{};
			waitInfo.m_Semaphore = pDevice->GetImageAvailableSemaphore();
			waitInfo.m_Stage = PipelineStage::ComputeShader;
			computeSubmitInfo.m_WaitSemaphores.push_back(waitInfo);
			computeSubmitInfo.m_SignalSemaphores.push_back(m_ComputeFinishedSemaphore);
			pDevice->SubmitComputeCommandList(compList, computeSubmitInfo);


			GraphicsCommandList gfxList = pDevice->GetGraphicsCmdList();
			gfxList.Begin();

			TextureBarrierDescription barrierDesc{};
			barrierDesc.m_Texture = pDevice->GetBackBuffer();
			barrierDesc.m_AspectMask = TextureAspectMask::Color;
			barrierDesc.m_SrcStage = PipelineStage::TopOfPipe;
			barrierDesc.m_SrcAccessMask = AccessMask::None;
			barrierDesc.m_DstStage = PipelineStage::ColorAttachmentOutput;
			barrierDesc.m_OldLayout = TextureLayout::Undefined;
			barrierDesc.m_NewLayout = TextureLayout::Color_Attachment;
			gfxList.Barrier(barrierDesc);

			gfxList.BeginRendering(RenderingInfo{
				pDevice->GetBackBufferSize(),
				{
					RenderingAttachment{
						pDevice->GetBackBufferView(),
						TextureLayout::Color_Attachment,
						LoadOp::Clear, StoreOp::Store
					}
				},
				false,
				RenderingAttachment{}
			});

			gfxList.SetDefaultViewportScissor(pDevice->GetBackBufferSize());
			gfxList.SetPipeline(m_GraphicsPipeline);
			gfxList.SetVertexBuffer(m_TriangleBuffer);
			gfxList.Draw(3, 1, 0, 0);

			gfxList.EndRendering();

			barrierDesc.m_Texture = pDevice->GetBackBuffer();
			barrierDesc.m_AspectMask = TextureAspectMask::Color;
			barrierDesc.m_SrcStage = PipelineStage::ColorAttachmentOutput;
			barrierDesc.m_SrcAccessMask = AccessMask::ColorAttachment_Write;
			barrierDesc.m_DstStage = PipelineStage::BottomOfPipe;
			barrierDesc.m_OldLayout = TextureLayout::Color_Attachment;
			barrierDesc.m_NewLayout = TextureLayout::Present_Src;
			gfxList.Barrier(barrierDesc);

			gfxList.End();

			CmdListSubmitInfo rendSubmitInfo{};
			rendSubmitInfo.m_SignalFence = pDevice->GetInFlightFence();
			rendSubmitInfo.m_SignalSemaphores.push_back(pDevice->GetRenderFinishedSemaphore());

			CmdListWaitSemaphoreInfo waitSemInfo{};
			waitSemInfo.m_Semaphore = m_ComputeFinishedSemaphore;
			waitSemInfo.m_Stage = PipelineStage::VertexInput;
			rendSubmitInfo.m_WaitSemaphores.push_back(waitSemInfo);

			pDevice->SubmitGraphicsCommandList(gfxList, rendSubmitInfo);

			pDevice->Present();
		}

		void TearDown(RenderDevice* pDevice) override {
			pDevice->DestroySemaphore(m_ComputeFinishedSemaphore);
			pDevice->DestroyBuffer(m_TriangleBuffer);
			pDevice->DestroyPipelineLayout(m_ComputeLayout);
			pDevice->DestroyPipeline(m_GraphicsPipeline);
			pDevice->DestroyPipeline(m_ComputePipeline);
		}
	};
}
