#pragma once
#include "CommonShapes.h"
#include "RenderSandboxEnv.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class TriangleTest : public CKE::IRenderSandboxTest
	{
	public:
		void Setup(RenderDevice* pDevice) override {
			BufferDesc bufferDesc{};
			bufferDesc.m_Name = "Triangle Buffer";
			bufferDesc.m_MemoryAccess = MemoryAccess::GPU;
			bufferDesc.m_SizeInBytes = sizeof(CommonShapes::m_TriangleVerts);
			bufferDesc.m_StrideInBytes = sizeof(TriangleVert);
			bufferDesc.m_Usage = BufferUsage::Vertex;
			bufferDesc.m_UpdateFrequency = UpdateFrequency::Static;
			m_TriangleBuffer = pDevice->CreateBuffer_DEPR(bufferDesc, CommonShapes::m_TriangleVerts.data(),
			                                              sizeof(CommonShapes::m_TriangleVerts));

			PipelineLayoutDesc layoutDesc{};
			m_PipelineLayout = pDevice->CreatePipelineLayout(layoutDesc);

			GraphicsPipelineDesc pipelineDesc{};
			pipelineDesc.m_VertexInput.SetVertexInput({
				VertexInputInfo{VertexInputFormat::Float_R32G32B32, 0},
				VertexInputInfo{VertexInputFormat::Float_R32G32B32, 0},
			});
			pipelineDesc.m_BlendState.m_AttachmentBlendStates = {AttachmentBlendState{}};
			pipelineDesc.m_AttachmentsInfo.m_ColorAttachments = {TextureFormat::B8G8R8A8_SRGB};
			pipelineDesc.m_DepthStencilState.m_DepthWrite = false;
			pipelineDesc.m_DepthStencilState.m_DepthTestEnable = false;
			pipelineDesc.m_DepthStencilState.m_StencilEnable = false;
			pipelineDesc.m_Topology = PrimitiveTopology::TriangleList;
			pipelineDesc.m_VertexShaderSource = g_FileSystem.ReadBinaryFile("test_01_vert.spv");
			pipelineDesc.m_FragmentShaderSource = g_FileSystem.ReadBinaryFile("test_01_frag.spv");
			pipelineDesc.m_LayoutHandle = m_PipelineLayout;
			m_Pipeline = pDevice->CreateGraphicsPipeline(pipelineDesc);
		}

		void Render(RenderDevice* pDevice) override {
			pDevice->AcquireNextBackBuffer();

			GraphicsCommandList cmdList = pDevice->GetGraphicsCmdList();
			cmdList.Begin();

			TextureBarrierDescription barrierDesc{};
			barrierDesc.m_Texture = pDevice->GetBackBuffer();
			barrierDesc.m_AspectMask = TextureAspectMask::Color;
			barrierDesc.m_SrcStage = PipelineStage::TopOfPipe;
			barrierDesc.m_SrcAccessMask = AccessMask::None;
			barrierDesc.m_DstStage = PipelineStage::ColorAttachmentOutput;
			barrierDesc.m_OldLayout = TextureLayout::Undefined;
			barrierDesc.m_NewLayout = TextureLayout::Color_Attachment;
			cmdList.Barrier(barrierDesc);

			cmdList.BeginRendering(RenderingInfo{
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

			cmdList.SetDefaultViewportScissor(pDevice->GetBackBufferSize());
			cmdList.SetPipeline(m_Pipeline);
			cmdList.SetVertexBuffer(m_TriangleBuffer);
			cmdList.Draw(3, 1, 0, 0);

			cmdList.EndRendering();

			barrierDesc.m_Texture = pDevice->GetBackBuffer();
			barrierDesc.m_AspectMask = TextureAspectMask::Color;
			barrierDesc.m_SrcStage = PipelineStage::ColorAttachmentOutput;
			barrierDesc.m_SrcAccessMask = AccessMask::ColorAttachment_Write;
			barrierDesc.m_DstStage = PipelineStage::BottomOfPipe;
			barrierDesc.m_OldLayout = TextureLayout::Color_Attachment;
			barrierDesc.m_NewLayout = TextureLayout::Present_Src;
			cmdList.Barrier(barrierDesc);

			cmdList.End();

			CmdListSubmitInfo rendSubmitInfo{};
			rendSubmitInfo.m_SignalFence = pDevice->GetInFlightFence();
			rendSubmitInfo.m_SignalSemaphores.push_back(pDevice->GetRenderFinishedSemaphore());
			CmdListWaitSemaphoreInfo waitSemInfo{};
			waitSemInfo.m_Semaphore = pDevice->GetImageAvailableSemaphore();
			waitSemInfo.m_Stage = PipelineStage::ColorAttachmentOutput;
			rendSubmitInfo.m_WaitSemaphores.push_back(waitSemInfo);
			pDevice->SubmitGraphicsCommandList(cmdList, rendSubmitInfo);

			pDevice->Present();
		}

		void TearDown(RenderDevice* pDevice) override {
			pDevice->DestroyBuffer(m_TriangleBuffer);
			pDevice->DestroyPipelineLayout(m_PipelineLayout);
			pDevice->DestroyPipeline(m_Pipeline);
		}

	private:
		BufferHandle         m_TriangleBuffer;
		PipelineLayoutHandle m_PipelineLayout;
		PipelineHandle       m_Pipeline;
	};
}
