#include "CookieKat/Engine/Render/RenderPasses/GBufferPass.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderingSystem.h"

#include "CookieKat/Engine/Entities/Components/LocalToWorldComponent.h"
#include "CookieKat/Engine/Entities/Components/MeshComponent.h"
#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"

#include "Common/GlobalRenderAssets.h"

#include <glm/gtc/type_ptr.hpp>

namespace CKE {
	void GBufferPass::Initialize(RenderPassInitCtx* pCtx) {
		m_pDevice = pCtx->GetDevice();
		m_pSamplerCache = pCtx->GetSamplerCache();
		m_pEntityDB = pCtx->GetEntityDatabase();
		m_pResources = pCtx->GetResourceSystem();
		m_pRenderingSettings = pCtx->GetRenderingSettings();
		m_Pipeline = pCtx->GetPipelineManager()->GetPipeline(PipelineIDS::GBufferPass);
	}

	void GBufferPass::Setup(FrameGraphSetupContext& setup) {
		setup.UseTexture(GBuffer::DepthStencil, FGPipelineAccessInfo::DepthStencil());
		setup.UseBuffer(SceneGlobal::View);
		setup.UseBuffer(SceneGlobal::ObjectData);

		// GBuffer Textures
		//-----------------------------------------------------------------------------

		TextureDesc texDesc{};
		texDesc.m_AspectMask = TextureAspectMask::Color;
		texDesc.m_TextureType = TextureType::Tex2D;
		TextureExtraSettings ext{true, {1.0f, 1.0f}};

		texDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		texDesc.m_DebugName = "GBuffer Albedo";
		setup.CreateTransientTexture(GBuffer::Albedo, texDesc, ext);
		setup.UseTexture(GBuffer::Albedo, FGPipelineAccessInfo::ColorAttachmentWrite());

		texDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		texDesc.m_DebugName = "GBuffer Normals";
		setup.CreateTransientTexture(GBuffer::Normals, texDesc, ext);
		setup.UseTexture(GBuffer::Normals, FGPipelineAccessInfo::ColorAttachmentWrite());

		texDesc.m_Format = TextureFormat::R32G32B32A32_SFLOAT;
		texDesc.m_DebugName = "GBuffer Positions";
		setup.CreateTransientTexture(GBuffer::Position, texDesc, ext);
		setup.UseTexture(GBuffer::Position, FGPipelineAccessInfo::ColorAttachmentWrite());

		texDesc.m_Format = TextureFormat::R8G8B8A8_UNORM;
		texDesc.m_DebugName = "GBuffer RoughMetalRefl";
		setup.CreateTransientTexture(GBuffer::RoughMetalRefl, texDesc, ext);
		setup.UseTexture(GBuffer::RoughMetalRefl, FGPipelineAccessInfo::ColorAttachmentWrite());

		// Additional Temporary
		//-----------------------------------------------------------------------------

		texDesc.m_Format = TextureFormat::R16G16B16A16_SFLOAT;
		texDesc.m_DebugName = "Object Idx";
		setup.CreateTransientTexture(GBuffer::ObjectIdx, texDesc, ext);
		setup.UseTexture(GBuffer::ObjectIdx, FGPipelineAccessInfo::ColorAttachmentWrite());
	}

	void GBufferPass::Execute(ExecuteResourcesCtx& ctx, CommandList& cmdList, RenderDevice& rd) {
		TextureViewHandle depthBufferTex = ctx.GetTextureView(GBuffer::DepthStencil);
		TextureViewHandle albedoTex = ctx.GetTextureView(GBuffer::Albedo);
		TextureViewHandle normalsTex = ctx.GetTextureView(GBuffer::Normals);
		TextureViewHandle positionTex = ctx.GetTextureView(GBuffer::Position);
		TextureViewHandle roughnessMetallicTex = ctx.GetTextureView(GBuffer::RoughMetalRefl);
		TextureViewHandle objIdxTex = ctx.GetTextureView(GBuffer::ObjectIdx);

		BufferHandle viewBuffer = ctx.GetBuffer(SceneGlobal::View);
		BufferHandle objectBuffer = ctx.GetBuffer(SceneGlobal::ObjectData);

		// Rendering Setup
		//-----------------------------------------------------------------------------

		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pRenderingSettings->m_RenderArea,
			.m_ColorAttachments = {
				RenderingAttachment{
					albedoTex, TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
				RenderingAttachment{
					positionTex,
					TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
				RenderingAttachment{
					normalsTex,
					TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
				RenderingAttachment{
					roughnessMetallicTex,
					TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
				RenderingAttachment{
					objIdxTex,
					TextureLayout::Color_Attachment,
					LoadOp::Clear, StoreOp::Store,
				},
			},
			.m_UseDepthAttachment = true,
			.m_DepthAttachment = RenderingAttachment{
				depthBufferTex,
				TextureLayout::DepthStencil_Attachment,
				LoadOp::Load, StoreOp::Store,
			},
		});

		cmdList.SetGraphicsPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pRenderingSettings->m_Viewport.m_Extent);

		// Global Descriptor
		//-----------------------------------------------------------------------------

		{
			DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
			auto                 globalDescriptor =
					b.BindUniformBuffer(0, viewBuffer)
					 .BindStorageBuffer(1, objectBuffer)
					 .Build();
			cmdList.BindDescriptor(m_Pipeline, globalDescriptor);
		}

		// Record draw calls
		//-----------------------------------------------------------------------------

		DescriptorSetBuilder b = rd.CreateDescriptorSetBuilder(m_Pipeline, 1);
		u64                  lastMaterialHandle = -1;
		for (auto& [l2w, mesh] : m_pEntityDB->GetMultiCompTupleIter<LocalToWorldComponent, MeshComponent>()) {

			// Skip over currently loading meshes
			if (!m_pResources->IsResourceLoaded(mesh->m_MeshID)) {
				continue;
			}

			// Initially set default textures
			TextureViewHandle albedo = GlobalRenderAssets::White1x1();
			TextureViewHandle normal = GlobalRenderAssets::NormalDefault();
			TextureViewHandle roughness = GlobalRenderAssets::White1x1();
			TextureViewHandle metallic = GlobalRenderAssets::White1x1();

			// Override default textures with material textures if any exist
			if (mesh->m_MaterialID.IsValid() && m_pResources->IsResourceLoaded(mesh->m_MaterialID)) {
				auto const material = m_pResources->GetResource<RenderMaterialResource>(mesh->m_MaterialID);
				if (material->GetAlbedoTexture().IsValid()) {
					albedo = m_pResources->GetResource<RenderTextureResource>(material->GetAlbedoTexture())->GetTextureView();
				}
				if (material->GetNormalTexture().IsValid()) {
					normal = m_pResources->GetResource<RenderTextureResource>(material->GetNormalTexture())->GetTextureView();
				}
				if (material->GetRoughnessTexture().IsValid()) {
					roughness = m_pResources->GetResource<RenderTextureResource>(material->GetRoughnessTexture())->
					                          GetTextureView();
				}
				if (material->GetMetalicTexture().IsValid()) {
					metallic = m_pResources->GetResource<RenderTextureResource>(material->GetMetalicTexture())->
					                         GetTextureView();
				}
			}

			// Material Bindings
			// Only bind if the material changed
			if (mesh->m_MaterialID.GetU64() != lastMaterialHandle) {
				SamplerDesc samplerDesc{};
				samplerDesc.m_WrapU = TextureWrapMode::Repeat;
				samplerDesc.m_WrapV = TextureWrapMode::Repeat;
				SamplerHandle       samplerHandle = m_pSamplerCache->CreateSampler(samplerDesc);
				DescriptorSetHandle materialDescriptor =
						b.BindTextureWithSampler(0, albedo, samplerHandle)
						 .BindTextureWithSampler(1, normal, samplerHandle)
						 .BindTextureWithSampler(2, roughness, samplerHandle)
						 .BindTextureWithSampler(3, metallic, samplerHandle)
						 .Build();

				cmdList.BindDescriptor(m_Pipeline, materialDescriptor);
				lastMaterialHandle = mesh->m_MaterialID.GetU64();
			}

			// Mesh Buffers
			MeshResource const* m = m_pResources->GetResource<MeshResource>(mesh->m_MeshID);
			cmdList.SetVertexBuffer(m->GetVertexBuffer());
			cmdList.SetIndexBuffer(m->GetIndexBuffer(), 0);
			cmdList.DrawIndexed(m->GetIndices().size(), mesh->m_ObjectIdx - 1);
		}

		cmdList.EndRendering();
	}
}
