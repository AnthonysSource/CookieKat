#include "CookieKat/Systems/FrameGraph/FrameGraph.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

#include "CookieKat/Core/Random/Random.h"
#include <CookieKat/Core/Logging/LoggingSystem.h>

namespace CKE {
	void FrameGraphSetupContext::UseTexture(FGResourceID id, FGPipelineAccessInfo accessInfo) {
		FGPassResourceUsageInfo usageInfo{};
		usageInfo.m_ID = id;
		usageInfo.m_Type = FGResourceType::Texture;
		usageInfo.m_AccessOp = FGResourceAccessOp::Read;
		m_ResourceUsageMetadata.push_back(usageInfo);

		FGTextureAccessInfo texAccessInfo{};
		texAccessInfo.m_ID = id;
		texAccessInfo.m_Access = accessInfo;
		m_TextureAccessInfo.push_back(texAccessInfo);
	}

	void FrameGraphSetupContext::UseBuffer(FGResourceID id) {
		FGPassResourceUsageInfo usageInfo{};
		usageInfo.m_ID = id;
		usageInfo.m_Type = FGResourceType::Buffer;
		usageInfo.m_AccessOp = FGResourceAccessOp::Read;
		m_ResourceUsageMetadata.push_back(usageInfo);
	}

	FGTextureAccessInfo FrameGraphSetupContext::GetTexAccessInfo(FGResourceID texID) {
		for (FGTextureAccessInfo const& tInfo : m_TextureAccessInfo) {
			if (texID == tInfo.m_ID) {
				return tInfo;
			}
		}
		CKE_UNREACHABLE_CODE();
		return FGTextureAccessInfo{};
	}

	void FrameGraphSetupContext::CreateTransientTexture(FGResourceID         id, TextureDesc desc,
	                                                    TextureExtraSettings extraSettings) {
		FGTexCreateInfo createInfo{};
		createInfo.m_ID = id;
		createInfo.m_Desc = desc;
		createInfo.m_Extra = extraSettings;
		m_CreateTextures.push_back(createInfo);
	}

	void FrameGraphSetupContext::CreateTransientBuffer(FGResourceID id, BufferDesc desc) {
		FGBuffCreateInfo createInfo{};
		createInfo.m_ID = id;
		createInfo.m_Desc = desc;
		m_CreateBuffers.push_back(createInfo);
	}
}

namespace CKE {
	void FrameGraphDB::AddTransientTexture(FGResourceID         fgID,
	                                       TextureHandle        texHandle,
	                                       TextureDesc          desc,
	                                       TextureViewHandle    viewHandle,
	                                       TextureExtraSettings extra) {
		FGResourceInfo resourceInfo{};
		resourceInfo.m_Type = FGResourceType::Texture;
		resourceInfo.m_Source = FGResourceSource::Transient;
		m_ResourceMetadata.insert({fgID, resourceInfo});

		FGTextureData fgTex{};
		fgTex.m_ID = fgID;
		fgTex.m_TexHandle = texHandle;
		fgTex.m_ViewHandle = viewHandle;
		fgTex.m_InitialAccess = FGPipelineAccessInfo{
			.m_Stage = PipelineStage::TopOfPipe,
			.m_Access = AccessMask::None,
			.m_Layout = TextureLayout::Undefined,
			.m_Aspect = desc.m_AspectMask,
			.m_LoadOp = LoadOp::DontCare
		};
		fgTex.m_TexDesc = desc;
		m_Textures.insert({fgID, fgTex});

		FGTransientTexture fgTransientTex{};
		fgTransientTex.m_Extra = extra;
		m_TransientTextures.insert({fgID, fgTransientTex});
	}

	void FrameGraphDB::AddImportedTexture(FGImportedTextureDesc desc) {
		FGResourceInfo resourceInfo{};
		resourceInfo.m_Type = FGResourceType::Texture;
		resourceInfo.m_Source = FGResourceSource::Imported;
		m_ResourceMetadata.insert({desc.m_fgID, resourceInfo});

		FGTextureData fgTex{};
		fgTex.m_ID = desc.m_fgID;
		fgTex.m_TexHandle = desc.m_TexHandle;
		fgTex.m_ViewHandle = desc.m_FullTexView;
		fgTex.m_InitialAccess = FGPipelineAccessInfo{
			.m_Stage = desc.m_SrcStageWhenAvailable,
			.m_Access = desc.m_SrcAccessMaskWhenAvailable,
			.m_Layout = desc.m_InitialLayout,
			.m_Aspect = desc.m_TexDesc.m_AspectMask,
			.m_LoadOp = desc.m_LoadOp
		};
		m_Textures.insert({desc.m_fgID, fgTex});

		FGImportedTexture fgTexture{};
		m_ImportedTextures.insert({desc.m_fgID, fgTexture});
	}

	void FrameGraphDB::AddTransientBuffer(FGResourceID fgID, BufferHandle handle, BufferDesc desc) {
		FGResourceInfo resourceInfo{};
		resourceInfo.m_Type = FGResourceType::Buffer;
		resourceInfo.m_Source = FGResourceSource::Transient;
		m_ResourceMetadata.insert({fgID, resourceInfo});

		FGBufferData fgBuff{};
		fgBuff.m_ID = fgID;
		fgBuff.m_Handle = handle;
		m_Buffers.insert({fgID, fgBuff});

		FGTransientBuffer fgTransientBuffer{};
		fgTransientBuffer.m_Desc = desc;
		m_TransientBuffers.insert({fgID, fgTransientBuffer});
	}

	void FrameGraphDB::AddImportedBuffer(FGResourceID fgID, BufferHandle handle, BufferDesc desc) {
		FGResourceInfo resourceInfo{};
		resourceInfo.m_Type = FGResourceType::Buffer;
		resourceInfo.m_Source = FGResourceSource::Imported;
		m_ResourceMetadata.insert({fgID, resourceInfo});

		FGBufferData fgBuff{};
		fgBuff.m_ID = fgID;
		fgBuff.m_Handle = handle;
		m_Buffers.insert({fgID, fgBuff});

		FGImportedBuffer fgImportedBuffer{};
		m_ImportedBuffers.insert({fgID, fgImportedBuffer});
	}

	void FrameGraphDB::RemoveTransientTexture(FGResourceID id) {
		CKE_ASSERT(m_Textures.contains(id));
		CKE_ASSERT(m_TransientTextures.contains(id));
		m_Textures.erase(id);
		m_TransientTextures.erase(id);
	}

	void FrameGraphDB::RemoveTransientBuffer(FGResourceID id) {
		CKE_ASSERT(m_Buffers.contains(id));
		CKE_ASSERT(m_TransientBuffers.contains(id));
		m_Buffers.erase(id);
		m_TransientBuffers.erase(id);
	}

	FGTextureData* FrameGraphDB::GetTexture(FGResourceID fgID) {
		return &m_Textures[fgID];
	}

	FGTransientTexture* FrameGraphDB::GetTransientTexture(FGResourceID fgID) {
		return &m_TransientTextures[fgID];
	}

	FGImportedTexture* FrameGraphDB::GetImportedTexture(FGResourceID fgID) {
		return &m_ImportedTextures[fgID];
	}

	FGBufferData* FrameGraphDB::GetBuffer(FGResourceID fgID) {
		return &m_Buffers[fgID];
	}

	FGTransientBuffer* FrameGraphDB::GetTransientBuffer(FGResourceID fgID) {
		return &m_TransientBuffers[fgID];
	}

	FGImportedBuffer* FrameGraphDB::GetImportedBuffer(FGResourceID fgID) {
		return &m_ImportedBuffers[fgID];
	}

	Vector<FGResourceID> FrameGraphDB::GetAllTransientTextures() {
		return MapUtils::VectorFromMapKeys(m_TransientTextures);
	}

	Vector<FGResourceID> FrameGraphDB::GetAllImportedTextures() {
		return MapUtils::VectorFromMapKeys(m_ImportedTextures);
	}

	Vector<FGResourceID> FrameGraphDB::GetAllTransientBuffers() {
		return MapUtils::VectorFromMapKeys(m_TransientBuffers);
	}

	bool FrameGraphDB::CheckImportedTextureExists(FGResourceID fgID) {
		return m_ImportedTextures.contains(fgID);
	}
}

namespace CKE {
	void FrameGraph::Initialize(RenderDevice* pDevice) {
		CKE_ASSERT(pDevice != nullptr);
		m_pDevice = pDevice;
	}

	void FrameGraph::Shutdown() {
		ClearCurrentCompilation();

		// Destroy all transient textures and buffers
		for (FGResourceID transientTexID : m_DB.GetAllTransientTextures()) {
			FGTextureData* pTexture = m_DB.GetTexture(transientTexID);
			m_pDevice->DestroyTextureView(pTexture->m_ViewHandle);
			m_pDevice->DestroyTexture(pTexture->m_TexHandle);
			// We can remove them while iterating because we iterate over a copy
			// of the IDs and not the actual resources.
			m_DB.RemoveTransientTexture(transientTexID);
		}

		for (FGResourceID transientBufferID : m_DB.GetAllTransientBuffers()) {
			FGBufferData* pBuffer = m_DB.GetBuffer(transientBufferID);
			m_pDevice->DestroyBuffer(pBuffer->m_Handle);
			m_DB.RemoveTransientBuffer(transientBufferID);
		}
	}

	void FrameGraph::AddPass(FGRenderPassID id, FGRenderPass* pPass, RenderPassType type) {
		CKE_ASSERT(pPass != nullptr);
		CKE_ASSERT(!m_RenderPassIDToIndex.contains(id));

		u64 passIndex = m_Passes.size() - 1;
		m_RenderPassIDToIndex.insert({id, passIndex});

		RenderPassData passData{};
		passData.m_Type = type;
		passData.m_pPass = pPass;
		m_Passes.push_back(passData);
	}

	void FrameGraph::AddGraphicsPass(FGGraphicsRenderPass* pRenderPass) {
		AddPass(pRenderPass->m_ID, pRenderPass, RenderPassType::Graphics);
	}

	void FrameGraph::AddTransferPass(FGTransferRenderPass* pRenderPass) {
		AddPass(pRenderPass->m_ID, pRenderPass, RenderPassType::Transfer);
	}

	void FrameGraph::AddComputePass(FGComputeRenderPass* pRenderPass) {
		AddPass(pRenderPass->m_ID, pRenderPass, RenderPassType::Compute);
	}

	void FrameGraph::CreateTransientResources(FrameGraphSetupContext setupCtx) {
		// Create all transient textures and buffers
		for (auto& [fgID, textureDesc, extra] : setupCtx.m_CreateTextures) {
			if (extra.m_UseSizeRelativeToRenderTarget) {
				textureDesc.m_Size = {
					m_RenderTargetSize.x * extra.m_RelativeSize.x,
					m_RenderTargetSize.y * extra.m_RelativeSize.y,
					1
				};
			}

			TextureHandle textureHandle = m_pDevice->CreateTexture(textureDesc);

			TextureViewDesc viewDesc{};
			viewDesc.m_Texture = textureHandle;
			viewDesc.m_Format = textureDesc.m_Format;
			viewDesc.m_Type = TextureViewType::Tex2D;
			viewDesc.m_AspectMask = textureDesc.m_AspectMask;
			if (static_cast<bool>(textureDesc.m_AspectMask & TextureAspectMask::Depth)) {
				// HACK
				viewDesc.m_AspectMask = TextureAspectMask::Depth;
			}
			TextureViewHandle viewHandle = m_pDevice->CreateTextureView(viewDesc);

			m_DB.AddTransientTexture(fgID, textureHandle, textureDesc, viewHandle, extra);
		}
		for (auto& [fgID, bufferDesc] : setupCtx.m_CreateBuffers) {
			BufferHandle bufferHandle = m_pDevice->CreateBuffer(bufferDesc);
			m_DB.AddTransientBuffer(fgID, bufferHandle, bufferDesc);
		}
	}

	FGTextureAccessInfo* FindInfoWithID(FGResourceID id, Vector<FGTextureAccessInfo>& vec) {
		for (FGTextureAccessInfo& tInfo : vec) {
			if (tInfo.m_ID == id) {
				return &tInfo;
			}
		}
		return nullptr;
	}

	void FrameGraph::ClearCurrentCompilation() {
		for (RenderPassData& node : m_Passes) {
			// Deffer the destruction of the semaphores so we are sure they are not in use
			for (SemaphoreHandle semaphoreHandle : node.m_SignalSemaphores) {
				m_SemaphoreDeletionList.push_back({
					RenderSettings::MAX_FRAMES_IN_FLIGHT, semaphoreHandle
				});
			}
			// Clear references
			node.ClearCompilation(m_pDevice);
		}
	}

	void FrameGraph::Compile_TransientTexUsageFlags() {
		struct UsageData
		{
			TextureUsage m_Usage{};
		};
		Map<FGResourceID, UsageData> usageDataMap{};

		// Store references to all of the transient textures to create
		for (RenderPassData& nodeData : m_Passes) {
			FGRenderPass*          pPass = (FGRenderPass*)nodeData.m_pPass;
			FrameGraphSetupContext setupCtx{};
			pPass->Setup(setupCtx);

			for (auto&& createInfo : setupCtx.m_CreateTextures) {
				if (!usageDataMap.contains(createInfo.m_ID)) {
					usageDataMap.insert({createInfo.m_ID, UsageData{}});
				}
			}
		}

		// For each pass, check if they access a transient texture, if so then
		// check the access mask and add the necessary usage to the current usages
		for (RenderPassData& nodeData : m_Passes) {
			FGRenderPass*          pPass = (FGRenderPass*)nodeData.m_pPass;
			FrameGraphSetupContext setupCtx{};
			pPass->Setup(setupCtx);

			for (auto&& accessInfo : setupCtx.m_TextureAccessInfo) {
				if (usageDataMap.contains(accessInfo.m_ID)) {
					TextureUsage usage = usageDataMap[accessInfo.m_ID].m_Usage;
					AccessMask   accessMask = accessInfo.m_Access.m_Access;

					if ((bool)(accessMask & AccessMask::Shader_Read) ||
						(bool)(accessMask & AccessMask::Shader_Write)) {
						usage = usage | TextureUsage::Sampled;
					}
					if ((bool)(accessMask & AccessMask::ColorAttachment_Read) ||
						(bool)(accessMask & AccessMask::ColorAttachment_Write)) {
						usage = usage | TextureUsage::Color_Attachment;
					}
					if ((bool)(accessMask & AccessMask::DepthStencilAttachment_Read) ||
						(bool)(accessMask & AccessMask::DepthStencilAttachment_Write)) {
						usage = usage | TextureUsage::DepthStencil_Attachment;
					}
					if ((bool)(accessMask & AccessMask::Transfer_Read)) {
						usage = usage | TextureUsage::Transfer_Src;
					}
					if ((bool)(accessMask & AccessMask::Transfer_Write)) {
						usage = usage | TextureUsage::Transfer_Dst;
					}

					usageDataMap[accessInfo.m_ID].m_Usage = usageDataMap[accessInfo.m_ID].m_Usage |
							usage;
				}
			}
		}

		// Update the transient texture descriptions and create them
		for (RenderPassData& nodeData : m_Passes) {
			FGRenderPass*          pPass = (FGRenderPass*)nodeData.m_pPass;
			FrameGraphSetupContext setupCtx{};
			pPass->Setup(setupCtx);

			for (auto&& createInfo : setupCtx.m_CreateTextures) {
				createInfo.m_Desc.m_Usage = usageDataMap[createInfo.m_ID].m_Usage;
				createInfo.m_Desc.m_ConcurrentQueueUsage = false;
			}

			CreateTransientResources(setupCtx);
		}
	}

	TextureBarrierDescription TexBarrierFromToAccess(FGPipelineAccessInfo src,
	                                                 FGPipelineAccessInfo dst,
	                                                 TextureHandle        texHandle,
	                                                 TextureAspectMask    aspectMask) {
		TextureBarrierDescription barrier{};
		barrier.m_SrcStage = src.m_Stage;
		barrier.m_SrcAccessMask = src.m_Access;
		barrier.m_DstStage = dst.m_Stage;
		barrier.m_DstAccessMask = dst.m_Access;
		barrier.m_OldLayout = src.m_Layout;
		barrier.m_NewLayout = dst.m_Layout;
		barrier.m_Texture = texHandle;
		barrier.m_AspectMask = aspectMask;

		return barrier;
	}

	FGTextureAccessInfo GetTexAccessInfo(Vector<FGTextureAccessInfo> const& textureAcesses, FGResourceID texID) {
		for (FGTextureAccessInfo const& tInfo : textureAcesses) {
			if (texID == tInfo.m_ID) {
				return tInfo;
			}
		}
		CKE_UNREACHABLE_CODE();
		return FGTextureAccessInfo{};
	}

	void FrameGraph::Compile_BarriersAndQueueSync() {
		// Contains info of the last usage of a resource
		struct ResourceTrackingInfo
		{
			RenderPassData*         m_pLastPass;
			FGPassResourceUsageInfo m_LastUsageInfo;
			FGTextureAccessInfo     m_LastTextureAccessInfo;
		};
		// Used to store all of the resources being used in a previously processed render pass
		Map<FGResourceID, ResourceTrackingInfo> resourceTrackingMap{};

		// Useful variables to keep track of for compilation
		bool            isFirstPassInGraph = true;
		RenderPassType  prevPassType = RenderPassType::Graphics;
		RenderPassData* pPrevPassData = nullptr;
		RenderPassData* pPrevGraphicsPass = nullptr;

		//-----------------------------------------------------------------------------

		for (RenderPassData& passData : m_Passes) {
			// Get resources accessed by the pass by calling its setup
			FGRenderPass*          pPass = (FGRenderPass*)passData.m_pPass;
			FrameGraphSetupContext setupCtx{};
			pPass->Setup(setupCtx);

			// Setup Initial References
			if (isFirstPassInGraph) {
				prevPassType = passData.m_Type;
				pPrevPassData = &passData;
				isFirstPassInGraph = false;
			}

			// If the previous pass uses a different queue we have to flush
			// it and create sync points with semaphores
			if (prevPassType != passData.m_Type) {
				// Previous Pass Signaling
				SemaphoreHandle queueSyncingSemaphore = m_pDevice->CreateSemaphoreGPU();
				pPrevPassData->m_SignalSemaphores.push_back(queueSyncingSemaphore);

				// Current Pass Waiting
				CmdListWaitSemaphoreInfo waitInfo{};
				waitInfo.m_Semaphore = queueSyncingSemaphore;
				if (passData.m_Type == RenderPassType::Graphics) { waitInfo.m_Stage = PipelineStage::TopOfPipe; } // Optimizable
				if (passData.m_Type == RenderPassType::Transfer) { waitInfo.m_Stage = PipelineStage::Transfer; }
				if (passData.m_Type == RenderPassType::Compute) { waitInfo.m_Stage = PipelineStage::ComputeShader; }
				passData.m_WaitSemaphores.push_back(waitInfo);

				pPrevPassData->m_SubmitAfterExecuting = true; // We have a queue switch so we flush the previous command batch
			}

			//-----------------------------------------------------------------------------

			for (FGPassResourceUsageInfo const& usageInfo : setupCtx.m_ResourceUsageMetadata) {
				// Check if we already saw this resource previously
				if (resourceTrackingMap.contains(usageInfo.m_ID)) {
					ResourceTrackingInfo& trackingInfo = resourceTrackingMap[usageInfo.m_ID];

					if (usageInfo.m_Type == FGResourceType::Texture) {
						// Get last and new access info
						FGTextureAccessInfo  textureAccessInfo = setupCtx.GetTexAccessInfo(usageInfo.m_ID);
						FGPipelineAccessInfo lastAcc = trackingInfo.m_LastTextureAccessInfo.m_Access;
						FGPipelineAccessInfo newAcc = textureAccessInfo.m_Access;
						trackingInfo.m_LastTextureAccessInfo = textureAccessInfo;

						// Create Texture Barrier Desc
						FGTextureData const* pTex = m_DB.GetTexture(usageInfo.m_ID);
						CKE_ASSERT(pTex->m_TexHandle != 0);
						TextureBarrierDescription barrierDesc = TexBarrierFromToAccess(
							lastAcc, newAcc, pTex->m_TexHandle, pTex->m_InitialAccess.m_Aspect);

						// Record Ownership Transfer

						// Filter out unnecessary read to read barriers
						bool keepBarrier = false;
						if (lastAcc.m_Layout == newAcc.m_Layout) {
							if (lastAcc.m_Layout != TextureLayout::Shader_ReadOnly &&
								lastAcc.m_Layout != TextureLayout::DepthStencil_ReadOnly) {
								keepBarrier = true;
							}
						}
						else {
							keepBarrier = true;
						}

						// TODO: We will assume for now that there is always a graphics pass at some point before a transfer one
						// If that is not the case then this is a nasty null ptr deref.
						if (passData.m_Type == RenderPassType::Transfer) {
							pPrevGraphicsPass->m_TransitionsAfter.push_back(barrierDesc);
						}
						else if (keepBarrier) {
							passData.m_TransitionsBefore.push_back(barrierDesc);
						}
					}
					else if (usageInfo.m_Type == FGResourceType::Buffer) {
						// TODO: We assume all buffer accesses are read-only
					}

					// Update Resource Tracking Info
					trackingInfo.m_pLastPass = &passData;
					trackingInfo.m_LastUsageInfo = usageInfo;
				}
				else {
					// This is the first time we see the resource so we start to track it
					ResourceTrackingInfo trackingInfo{};
					trackingInfo.m_pLastPass = &passData;
					trackingInfo.m_LastUsageInfo = usageInfo;

					if (usageInfo.m_Type == FGResourceType::Texture) {
						FGTextureAccessInfo  textureAccessInfo = setupCtx.GetTexAccessInfo(usageInfo.m_ID);
						FGPipelineAccessInfo newAcc = textureAccessInfo.m_Access;
						trackingInfo.m_LastTextureAccessInfo = textureAccessInfo;

						// Create barrier from initial to new required layout
						FGTextureData* pTex = m_DB.GetTexture(usageInfo.m_ID);
						CKE_ASSERT(pTex->m_TexHandle != 0);
						TextureBarrierDescription barrierDesc = TexBarrierFromToAccess(
							pTex->m_InitialAccess, newAcc, pTex->m_TexHandle, pTex->m_InitialAccess.m_Aspect);
						passData.m_TransitionsBefore.push_back(barrierDesc);
					}

					resourceTrackingMap.insert({usageInfo.m_ID, trackingInfo});
				}
			}

			// Update Compilation Tracking Data
			prevPassType = passData.m_Type;
			pPrevPassData = &passData;
			if (passData.m_Type == RenderPassType::Graphics) {
				pPrevGraphicsPass = &passData;
			}

			// Setup RenderPassContext data that can be accessed from inside the render pass
			passData.m_ExecuteContext.PopulateContext(setupCtx.m_ResourceUsageMetadata, &m_DB);
		}
	}

	// Adds the already calculated barriers for the resources to the command list
	void FrameGraph::RecordResouceTransitions(GraphicsCommandList& cmdList, RenderPassData& renderPass) {
		cmdList.Barrier(renderPass.m_TransitionsBefore);
	}

	SemaphoreHandle i_PassExecutionFinishedSemaphore{};

	void FrameGraph::Compile(UInt2 renderTargetSize) {
		m_RenderTargetSize = renderTargetSize;
		i_PassExecutionFinishedSemaphore = m_pDevice->CreateSemaphoreGPU();

		ClearCurrentCompilation();
		Compile_TransientTexUsageFlags();
		Compile_BarriersAndQueueSync();
	}

	void FrameGraph::Execute_ReturnTexturesToOriginal(SemaphoreHandle signalSemaphoreOnFinish, FenceHandle signalFenceOnFinish) {
		// Return Imported Textures to initial state
		GraphicsCommandList revertLayoutsCmdList = m_pDevice->GetGraphicsCmdList();
		revertLayoutsCmdList.Begin();

		for (FGResourceID const fgID : m_DB.GetAllImportedTextures()) {
			if (fgID == "Swapchain") {
				continue;
			}

			FGTextureData* pTex = m_DB.GetTexture(fgID);

			// Find the last resouce state by checking the last barrier Dst
			// TODO: This is quite oof, pls fix
			TextureBarrierDescription lastBarrier{};
			for (RenderPassData& pass : m_Passes) {
				for (TextureBarrierDescription& barrier : pass.m_TransitionsBefore) {
					if (barrier.m_Texture == pTex->m_TexHandle) {
						lastBarrier = barrier;
					}
				}
			}

			// Transition the texture from the state at the end of the execution
			// to the necessary state at the beginning of it
			revertLayoutsCmdList.Barrier(TextureBarrierDescription{
				.m_SrcStage = lastBarrier.m_DstStage,
				.m_SrcAccessMask = lastBarrier.m_DstAccessMask,
				.m_DstStage = pTex->m_InitialAccess.m_Stage,
				.m_DstAccessMask = pTex->m_InitialAccess.m_Access,
				.m_OldLayout = lastBarrier.m_NewLayout,
				.m_NewLayout = pTex->m_InitialAccess.m_Layout,
				.m_Texture = pTex->m_TexHandle,
				.m_AspectMask = pTex->m_InitialAccess.m_Aspect
			});
		}
		revertLayoutsCmdList.End();

		CmdListSubmitInfo        revertLayoutsSubmInfo{};
		CmdListWaitSemaphoreInfo wait1{};
		wait1.m_Semaphore = i_PassExecutionFinishedSemaphore;
		wait1.m_Stage = PipelineStage::BottomOfPipe;
		revertLayoutsSubmInfo.m_WaitSemaphores.push_back(wait1);
		revertLayoutsSubmInfo.m_SignalSemaphores.push_back(signalSemaphoreOnFinish);
		revertLayoutsSubmInfo.m_SignalFence = signalFenceOnFinish;
		m_pDevice->SubmitGraphicsCommandList(revertLayoutsCmdList, revertLayoutsSubmInfo);

		// Update the deletion of leftover semaphores from previous compilations
		Vector<DeletionEntry> temp{};
		for (DeletionEntry& entry : m_SemaphoreDeletionList) {
			if (entry.m_FramesTillDeletion <= 0) {
				m_pDevice->DestroySemaphore(entry.m_Handle);
			}
			else {
				entry.m_FramesTillDeletion--;
				temp.emplace_back(entry);
			}
		}
		m_SemaphoreDeletionList = temp;
	}

	template <typename T>
	struct QueueRecordingState
	{
		bool m_IsFirstPassOverall = true;
		bool m_CurrentlyRecording = false;
		T    m_CmdList;
	};

	Vec3 RandomColor() {
		return Vec3{Random::F32(0, 1), Random::F32(0, 1), Random::F32(0, 1)};
	}

	void FrameGraph::Execute(CmdListWaitSemaphoreInfo waitInfoAtStart,
	                         SemaphoreHandle          signalSemaphoreOnFinish,
	                         FenceHandle              signalFenceOnFinish) {
		// The last pass must always be submitted and sync
		RenderPassData& finalRenderPassData = m_Passes[m_Passes.size() - 1];
		finalRenderPassData.m_SignalSemaphores.clear(); // TODO: Not great to just clear all
		finalRenderPassData.m_SignalSemaphores.push_back(i_PassExecutionFinishedSemaphore);
		finalRenderPassData.m_SubmitAfterExecuting = true;

		//-----------------------------------------------------------------------------

		QueueRecordingState<GraphicsCommandList> gfxState{};
		QueueRecordingState<TransferCommandList> transfState{};
		QueueRecordingState<ComputeCommandList>  compState{};

		bool isFirstPassOverall = true;

		for (RenderPassData& passData : m_Passes) {
			passData.m_ExecuteContext.RefreshContext(&m_DB);

			if (passData.m_Type == RenderPassType::Graphics) {
				auto pPass = (FGGraphicsRenderPass*)passData.m_pPass;

				if (!gfxState.m_CurrentlyRecording) {
					gfxState.m_CurrentlyRecording = true;
					gfxState.m_CmdList = m_pDevice->GetGraphicsCmdList();
					gfxState.m_CmdList.Begin();
				}

				gfxState.m_CmdList.BeginDebugLabel(pPass->m_ID.c_str(), RandomColor());
				RecordResouceTransitions(gfxState.m_CmdList, passData);
				pPass->Execute(passData.m_ExecuteContext, gfxState.m_CmdList, *m_pDevice);
				gfxState.m_CmdList.EndDebugLabel();

				if (passData.m_SubmitAfterExecuting) {
					gfxState.m_CmdList.End();
					CmdListSubmitInfo submitInfo = passData.GetSubmitInfo();

					if (isFirstPassOverall) {
						submitInfo.m_WaitSemaphores.push_back(waitInfoAtStart);
						isFirstPassOverall = false;
					}

					m_pDevice->SubmitGraphicsCommandList(gfxState.m_CmdList, submitInfo);
					gfxState.m_CurrentlyRecording = false;
				}
			}
			else if (passData.m_Type == RenderPassType::Transfer) {
				auto pPass = (FGTransferRenderPass*)passData.m_pPass;

				if (!transfState.m_CurrentlyRecording) {
					transfState.m_CurrentlyRecording = true;
					transfState.m_CmdList = m_pDevice->GetTransferCmdList();
					transfState.m_CmdList.Begin();
				}

				transfState.m_CmdList.BeginDebugLabel(pPass->m_ID.c_str(), RandomColor());
				pPass->Execute(passData.m_ExecuteContext, transfState.m_CmdList, *m_pDevice);
				transfState.m_CmdList.EndDebugLabel();

				if (passData.m_SubmitAfterExecuting) {
					transfState.m_CmdList.End();
					CmdListSubmitInfo submitInfo = passData.GetSubmitInfo();

					if (isFirstPassOverall) {
						waitInfoAtStart.m_Stage = PipelineStage::Transfer;
						submitInfo.m_WaitSemaphores.push_back(waitInfoAtStart);
						isFirstPassOverall = false;
					}

					m_pDevice->SubmitTransferCommandList(transfState.m_CmdList, submitInfo);
					transfState.m_CurrentlyRecording = false;
				}
			}
			else if (passData.m_Type == RenderPassType::Compute) {
				auto pPass = (FGComputeRenderPass*)passData.m_pPass;

				if (!compState.m_CurrentlyRecording) {
					compState.m_CurrentlyRecording = true;
					compState.m_CmdList = m_pDevice->GetComputeCmdList();
					compState.m_CmdList.Begin();
				}

				compState.m_CmdList.BeginDebugLabel(pPass->m_ID.c_str(), RandomColor());
				pPass->Execute(passData.m_ExecuteContext, compState.m_CmdList, *m_pDevice);
				compState.m_CmdList.EndDebugLabel();

				if (passData.m_SubmitAfterExecuting) {
					compState.m_CmdList.End();
					CmdListSubmitInfo submitInfo = passData.GetSubmitInfo();

					if (isFirstPassOverall) {
						waitInfoAtStart.m_Stage = PipelineStage::ComputeShader;
						submitInfo.m_WaitSemaphores.push_back(waitInfoAtStart);
						isFirstPassOverall = false;
					}

					m_pDevice->SubmitComputeCommandList(compState.m_CmdList, submitInfo);
					compState.m_CurrentlyRecording = false;
				}
			}
		}

		//-----------------------------------------------------------------------------

		Execute_ReturnTexturesToOriginal(signalSemaphoreOnFinish, signalFenceOnFinish);
	}

	void FrameGraph::ImportTexture(FGImportedTextureDesc desc) {
		// Check if the texture exists first, if it does, update it,
		// if it doesn't then import it
		if (m_DB.CheckImportedTextureExists(desc.m_fgID)) {
			FGTextureData* pTex = m_DB.GetTexture(desc.m_fgID);

			// We have to update the already compiled barriers
			bool foundFirst = false;
			for (RenderPassData& pass : m_Passes) {
				for (TextureBarrierDescription& barrier : pass.m_TransitionsBefore) {
					if (barrier.m_Texture == pTex->m_TexHandle) {
						// We update all of the texture handles
						barrier.m_Texture = desc.m_TexHandle;

						// In the first barrier we have we must update
						// the initial state
						if (!foundFirst) {
							barrier.m_OldLayout = desc.m_InitialLayout;
							barrier.m_SrcStage = desc.m_SrcStageWhenAvailable;
							barrier.m_SrcAccessMask = desc.m_SrcAccessMaskWhenAvailable;
							foundFirst = true;
						}
					}
				}
			}

			pTex->m_TexHandle = desc.m_TexHandle;
		}
		else {
			m_DB.AddImportedTexture(desc);
		}

		// Transition to required layout
		auto cmdList = m_pDevice->GetGraphicsCmdList();
		cmdList.Begin();
		cmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::AllGraphics,
			.m_SrcAccessMask = AccessMask::None,
			.m_DstStage = PipelineStage::AllGraphics,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Undefined,
			.m_NewLayout = desc.m_InitialLayout,
			.m_Texture = desc.m_TexHandle,
			.m_AspectMask = desc.m_TexDesc.m_AspectMask
		});
		cmdList.End();
		m_pDevice->SubmitGraphicsCommandList(cmdList, {});
		m_pDevice->WaitGraphicsQueueIdle();
	}

	void FrameGraph::ImportBuffer(FGResourceID fgID, BufferHandle bufferHandle) {
		m_DB.AddImportedBuffer(fgID, bufferHandle, {});
	}

	void FrameGraph::UpdateImportedTexture(FGImportedTextureDesc desc) {
		// Check if the texture exists first, if it does, update it,
		// if it doesn't then create a new one
		TextureLayout     initialLayout{};
		TextureAspectMask initialAspect{};

		if (m_DB.CheckImportedTextureExists(desc.m_fgID)) {
			FGTextureData* pTex = m_DB.GetTexture(desc.m_fgID);
			initialLayout = pTex->m_InitialAccess.m_Layout;
			initialAspect = pTex->m_InitialAccess.m_Aspect;

			bool foundFirst = false;
			for (RenderPassData& pass : m_Passes) {
				for (TextureBarrierDescription& barrier : pass.m_TransitionsBefore) {
					if (barrier.m_Texture == pTex->m_TexHandle) {
						// We update all of the texture handles
						barrier.m_Texture = desc.m_TexHandle;

						if (!foundFirst) {
							initialLayout = barrier.m_OldLayout;
							initialAspect = barrier.m_AspectMask;
							foundFirst = true;
						}
					}
				}
			}

			pTex->m_TexHandle = desc.m_TexHandle;
			pTex->m_ViewHandle = desc.m_FullTexView;
			pTex->m_TexDesc = desc.m_TexDesc;
		}
		else {
			CKE_UNREACHABLE_CODE();
		}

		// Transition to the required initial layout
		auto cmdList = m_pDevice->GetGraphicsCmdList();
		cmdList.Begin();
		cmdList.Barrier(TextureBarrierDescription{
			.m_SrcStage = PipelineStage::AllCommands,
			.m_SrcAccessMask = AccessMask::None,
			.m_DstStage = PipelineStage::AllCommands,
			.m_DstAccessMask = AccessMask::None,
			.m_OldLayout = TextureLayout::Undefined,
			.m_NewLayout = initialLayout,
			.m_Texture = desc.m_TexHandle,
			.m_AspectMask = initialAspect
		});
		cmdList.End();
		m_pDevice->SubmitGraphicsCommandList(cmdList, {});
		m_pDevice->WaitGraphicsQueueIdle();
	}

	void FrameGraph::UpdateRenderTargetSize(UInt2 newSize) {
		if (newSize == m_RenderTargetSize) {
			return;
		}

		m_RenderTargetSize = newSize;

		// Go through all of the textures with size relative to the swapchain
		// and recreate them with the appropriate new size
		for (FGResourceID texID : m_DB.GetAllTransientTextures()) {
			FGTextureData*      t1 = m_DB.GetTexture(texID);
			FGTransientTexture* t2 = m_DB.GetTransientTexture(texID);

			if (t2->m_Extra.m_UseSizeRelativeToRenderTarget) {
				m_pDevice->DestroyTexture(t1->m_TexHandle);

				// Calculate new relative size and create the texture + its full view
				t1->m_TexDesc.m_Size = {
					m_RenderTargetSize.x * t2->m_Extra.m_RelativeSize.x,
					m_RenderTargetSize.y * t2->m_Extra.m_RelativeSize.y,
					1
				};
				TextureHandle newHandle = m_pDevice->CreateTexture(t1->m_TexDesc);

				TextureViewDesc viewDesc{};
				viewDesc.m_Format = t1->m_TexDesc.m_Format;
				viewDesc.m_Type = TextureViewType::Tex2D;
				viewDesc.m_AspectMask = t1->m_TexDesc.m_AspectMask;
				viewDesc.m_BaseArrayLayer = 0;
				viewDesc.m_ArrayLayerCount = t1->m_TexDesc.m_ArraySize;
				viewDesc.m_BaseMipLevel = 0;
				viewDesc.m_MipLevelCount = t1->m_TexDesc.m_MipLevels;
				viewDesc.m_Texture = newHandle;
				if (static_cast<bool>(viewDesc.m_AspectMask & TextureAspectMask::Depth)) {
					viewDesc.m_AspectMask = TextureAspectMask::Depth;
				}
				TextureViewHandle viewHandle = m_pDevice->CreateTextureView(viewDesc);

				// We have to update the already compiled barriers
				for (RenderPassData& pass : m_Passes) {
					for (TextureBarrierDescription& barrier : pass.m_TransitionsBefore) {
						if (barrier.m_Texture == t1->m_TexHandle) {
							barrier.m_Texture = newHandle;
						}
					}
				}

				// Update the transient texture
				t1->m_TexHandle = newHandle;
				t1->m_ViewHandle = viewHandle;
			}
		}
	}
}
