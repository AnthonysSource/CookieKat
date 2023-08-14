#pragma once

#include "CommonShapes.h"
#include "RenderSandboxEnv.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/FrameGraph/FrameGraph.h"

namespace CKE {
	class FrameGraph_ComputeVertex : public IRenderSandboxTest
	{
	public:
		class TestComputePass : public FGComputeRenderPass
		{
		public:
			PipelineHandle m_ComputePipeline;

			TestComputePass(PipelineHandle mComputePipeline)
				: FGComputeRenderPass{"TestComputePass"},
				  m_ComputePipeline{mComputePipeline} {}

			void Setup(FrameGraphSetupContext& setup) override {
				setup.UseBuffer("Vertex");
			}

			void Execute(ExecuteResourcesCtx& ctx, ComputeCommandList& cmdList, RenderDevice& rd) override {
				BufferHandle vertex = ctx.GetBuffer("Vertex");

				DescriptorSetBuilder builder = rd.CreateDescriptorSetBuilder(m_ComputePipeline, 0);
				DescriptorSetHandle  setHandle = builder.BindStorageBuffer(0, vertex).Build();
				cmdList.BindComputeDescriptor(m_ComputePipeline, setHandle);
				cmdList.SetComputePipeline(m_ComputePipeline);
				cmdList.Dispatch(3, 1, 1);
			}
		};

		class TestDrawPass : public FGGraphicsRenderPass
		{
		public:
			PipelineHandle m_GraphicsPipeline;

			TestDrawPass(PipelineHandle mGraphicsPipeline) :
				FGGraphicsRenderPass{"TestDrawPass"},
				m_GraphicsPipeline{mGraphicsPipeline} {}

			void Setup(FrameGraphSetupContext& setup) override {
				setup.UseBuffer("Vertex");
			}

			void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override {
				BufferHandle              vertex = ctx.GetBuffer("Vertex");
				TextureBarrierDescription barrierDesc{};
				barrierDesc.m_Texture = rd.GetBackBuffer();
				barrierDesc.m_AspectMask = TextureAspectMask::Color;
				barrierDesc.m_SrcStage = PipelineStage::TopOfPipe;
				barrierDesc.m_SrcAccessMask = AccessMask::None;
				barrierDesc.m_DstStage = PipelineStage::ColorAttachmentOutput;
				barrierDesc.m_OldLayout = TextureLayout::Undefined;
				barrierDesc.m_NewLayout = TextureLayout::Color_Attachment;
				cmdList.Barrier(barrierDesc);

				cmdList.BeginRendering(RenderingInfo{
					rd.GetBackBufferSize(),
					{
						RenderingAttachment{
							rd.GetBackBufferView(),
							TextureLayout::Color_Attachment,
							LoadOp::Clear, StoreOp::Store
						}
					},
					false,
					RenderingAttachment{}
				});

				cmdList.SetDefaultViewportScissor(rd.GetBackBufferSize());
				cmdList.SetPipeline(m_GraphicsPipeline);
				cmdList.SetVertexBuffer(vertex);
				cmdList.Draw(3, 1, 0, 0);

				cmdList.EndRendering();

				barrierDesc.m_Texture = rd.GetBackBuffer();
				barrierDesc.m_AspectMask = TextureAspectMask::Color;
				barrierDesc.m_SrcStage = PipelineStage::ColorAttachmentOutput;
				barrierDesc.m_SrcAccessMask = AccessMask::ColorAttachment_Write;
				barrierDesc.m_DstStage = PipelineStage::BottomOfPipe;
				barrierDesc.m_OldLayout = TextureLayout::Color_Attachment;
				barrierDesc.m_NewLayout = TextureLayout::Present_Src;
				cmdList.Barrier(barrierDesc);
			}
		};

	public:
		FrameGraph           m_FrameGraph;
		PipelineLayoutHandle m_GfxLayout;
		PipelineLayoutHandle m_ComputeLayout;
		PipelineHandle       m_GraphicsPipeline;
		PipelineHandle       m_ComputePipeline;
		BufferHandle         m_TriangleBuffer;
		TestComputePass*     m_pComputePass;
		TestDrawPass*        m_pGfxPass;

	public:
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

			m_FrameGraph.Initialize(pDevice);

			//FGImportedTextureDesc swapChainDesc{};
			//swapChainDesc.m_fgID = "Swapchain";
			//swapChainDesc.m_TexHandle = pDevice->GetBackBuffer();
			//swapChainDesc.m_TexDesc = pDevice->GetBackBufferDesc();
			//swapChainDesc.m_FullTexView = pDevice->GetBackBufferView();
			//swapChainDesc.m_InitialLayout = TextureLayout::Present_Src;
			//swapChainDesc.m_SrcStageWhenAvailable = PipelineStage::BottomOfPipe;
			//swapChainDesc.m_SrcAccessMaskWhenAvailable = AccessMask::None;
			//swapChainDesc.m_LoadOp = LoadOp::DontCare;
			//m_FrameGraph.ImportTexture(swapChainDesc);

			m_FrameGraph.ImportBuffer("Vertex", m_TriangleBuffer);

			m_pComputePass = new TestComputePass(m_ComputePipeline);
			m_pGfxPass = new TestDrawPass(m_GraphicsPipeline);
			m_FrameGraph.AddComputePass(m_pComputePass);
			m_FrameGraph.AddGraphicsPass(m_pGfxPass);
			m_FrameGraph.Compile(pDevice->GetBackBufferSize());
		}

		void Render(RenderDevice* pDevice) override {
			pDevice->AcquireNextBackBuffer();

			CmdListWaitSemaphoreInfo waitSemInfo{};
			waitSemInfo.m_Semaphore = pDevice->GetImageAvailableSemaphore();
			waitSemInfo.m_Stage = PipelineStage::ColorAttachmentOutput;
			m_FrameGraph.Execute(waitSemInfo, pDevice->GetRenderFinishedSemaphore(), pDevice->GetInFlightFence());

			pDevice->Present();
		}

		void TearDown(RenderDevice* pDevice) override {
			m_FrameGraph.Shutdown();
			pDevice->DestroyPipelineLayout(m_ComputeLayout);
			pDevice->DestroyPipelineLayout(m_GfxLayout);
			pDevice->DestroyPipeline(m_GraphicsPipeline);
			pDevice->DestroyPipeline(m_ComputePipeline);
			pDevice->DestroyBuffer(m_TriangleBuffer);
		}
	};
}
