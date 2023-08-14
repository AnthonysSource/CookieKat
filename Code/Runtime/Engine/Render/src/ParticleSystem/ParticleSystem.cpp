#include "CookieKat/Engine/Render/ParticleSystem/ParticleSystem.h"

#include "CookieKat/Core/Random/Random.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	void ParticleSystem::Initialize(RenderDevice* pDevice) {
		m_pDevice = pDevice;

		// Create the initial data for the particles
		std::default_random_engine          rndEngine(time(nullptr));
		std::uniform_real_distribution<f32> posDist(-1.0f, 1.0f);
		std::uniform_real_distribution<f32> colorDist(0.0f, 1.0f);
		u32                                 spawnWidth = 16;
		for (i32 y = 0; y < spawnWidth; ++y) {
			for (i32 x = 0; x < spawnWidth; ++x) {
				i32             idx = y * spawnWidth + x;
				ParticleDataGPU particleData;
				particleData.m_Pos = Vec4{x - spawnWidth / 2.0f, y - spawnWidth / 2.0f, 0.0f, 0.0f};
				particleData.m_Velocity = Vec4{posDist(rndEngine), posDist(rndEngine), posDist(rndEngine), 0.0f};
				particleData.m_Color = Vec4{colorDist(rndEngine), colorDist(rndEngine), colorDist(rndEngine), 0.0f};
				particleData.m_Velocity = Vec4{0.5f, 1.0f, 0.5, 0.0f};
				m_ParticlesInitialData[idx] = particleData;
			}
		}

		// Create and upload particles to GPU
		BufferDesc bufferDesc{};
		bufferDesc.m_Name = "Particle Buffer";
		bufferDesc.m_MemoryAccess = MemoryAccess::GPU;
		bufferDesc.m_SizeInBytes = sizeof(m_ParticlesInitialData);
		bufferDesc.m_StrideInBytes = sizeof(m_ParticlesInitialData[0]);
		bufferDesc.m_Usage = BufferUsage::Storage | BufferUsage::TransferDst;
		bufferDesc.m_UpdateFrequency = UpdateFrequency::Static;
		m_ParticlesCurrent = m_pDevice->CreateBuffer_DEPR(bufferDesc, m_ParticlesInitialData.data(), sizeof(m_ParticlesInitialData));
		m_ParticlesLast = m_pDevice->CreateBuffer_DEPR(bufferDesc, m_ParticlesInitialData.data(), sizeof(m_ParticlesInitialData));

		// Create compute pipeline
		PipelineLayoutDesc layoutDesc{};
		layoutDesc.SetShaderBindings({
				ShaderBinding{0, 0, ShaderBindingType::StorageBuffer, 1, ShaderStageMask::Compute},
				ShaderBinding{0, 1, ShaderBindingType::StorageBuffer, 1, ShaderStageMask::Compute}
			}
		);
		PipelineLayoutHandle layoutHandle = m_pDevice->CreatePipelineLayout(layoutDesc);
		ComputePipelineDesc  pipelineDesc;
		pipelineDesc.m_Layout = layoutHandle;
		pipelineDesc.m_ComputeShaderSrc = g_FileSystem.ReadBinaryFile("../../../../Data/particles.spirv");
		m_ComputePipeline = m_pDevice->CreateComputePipeline(pipelineDesc);

		// Create graphics pipeline
		GraphicsPipelineDesc gfxPipelineDesc{};
		gfxPipelineDesc.m_BlendState.m_AttachmentBlendStates.push_back(AttachmentBlendState{});
		gfxPipelineDesc.m_DepthStencilState.m_DepthWrite = false;
		gfxPipelineDesc.m_DepthStencilState.m_DepthTestEnable = false;
		gfxPipelineDesc.m_DepthStencilState.m_DepthCompareOp = CompareOp::LessOrEqual;
		gfxPipelineDesc.m_AttachmentsInfo.m_ColorAttachments.push_back(TextureFormat::B8G8R8A8_SRGB);
		gfxPipelineDesc.m_AttachmentsInfo.m_DepthStencil = TextureFormat::D24_UNORM_S8_UINT;
		gfxPipelineDesc.m_VertexInput.SetVertexInput({
			VertexInputInfo{VertexInputFormat::Float_R32G32B32},
			VertexInputInfo{VertexInputFormat::Float_R32G32B32},
			VertexInputInfo{VertexInputFormat::Float_R32G32B32},
		});
		gfxPipelineDesc.m_Topology = PrimitiveTopology::TriangleList;
		gfxPipelineDesc.m_VertexShaderSource = g_FileSystem.ReadBinaryFile("../../../../Data/particlesVert.spirv");
		gfxPipelineDesc.m_FragmentShaderSource = g_FileSystem.ReadBinaryFile("../../../../Data/particlesFrag.spirv");
		m_GfxPipeline = m_pDevice->CreateGraphicsPipeline(gfxPipelineDesc);
	}

	void ParticleSystem::Update() {
		// Compute Particles
		ComputeCommandList cmdList = m_pDevice->GetComputeCmdList();
		cmdList.Begin();
		cmdList.BeginDebugLabel("Particle System", Vec3{1.0f, 0.0f, 1.0f});

		auto descriptorBuider = m_pDevice->CreateDescriptorSetBuilder(m_ComputePipeline, 0);
		descriptorBuider.BindStorageBuffer(0, m_ParticlesLast);
		descriptorBuider.BindStorageBuffer(1, m_ParticlesCurrent);
		DescriptorSetHandle descriptorHandle = descriptorBuider.Build();

		cmdList.BindComputeDescriptor(m_ComputePipeline, descriptorHandle);
		cmdList.SetComputePipeline(m_ComputePipeline);
		cmdList.Dispatch(256, 1, 1);

		cmdList.EndDebugLabel();
		cmdList.End();

		CmdListSubmitInfo computeSubmitInfo{};
		m_pDevice->SubmitComputeCommandList(cmdList, CmdListSubmitInfo{});

		m_pDevice->WaitForDevice();

		// Draw Particles
		GraphicsCommandList gfxList = m_pDevice->GetGraphicsCmdList();
		gfxList.Begin();
		gfxList.BeginDebugLabel("Particle Draw", Vec3{0.0f, 1.0f, 0.0f});

		RenderingInfo renderingInfo{};
		renderingInfo.m_RenderArea = m_pDevice->GetBackBufferSize();
		RenderingAttachment colorAttach{};
		colorAttach.m_Layout = TextureLayout::Color_Attachment;
		colorAttach.m_ClearValue = Vec4{ 0.0f };
		colorAttach.m_LoadOp = LoadOp::Clear;
		colorAttach.m_StoreOp = StoreOp::Store;
		colorAttach.m_TextureView = m_pDevice->GetBackBufferView();
		renderingInfo.m_ColorAttachments.push_back(colorAttach);
		gfxList.BeginRendering(renderingInfo);

		gfxList.SetPipeline(m_GfxPipeline);
		gfxList.SetVertexBuffer(m_ParticlesCurrent);
		gfxList.Draw(256, 1, 0, 0);

		gfxList.EndRendering();

		gfxList.EndDebugLabel();
		gfxList.End();

		m_pDevice->SubmitGraphicsCommandList(gfxList, CmdListSubmitInfo{});
		m_pDevice->WaitForDevice();

		// Switch buffers for next frame
		BufferHandle temp = m_ParticlesLast;
		m_ParticlesLast = m_ParticlesCurrent;
		m_ParticlesCurrent = temp;
	}

	void ParticleSystem::Shutdown() {
		m_pDevice->DestroyBuffer(m_ParticlesCurrent);
		m_pDevice->DestroyBuffer(m_ParticlesLast);
		m_pDevice->DestroyPipeline(m_ComputePipeline);
	}
}
