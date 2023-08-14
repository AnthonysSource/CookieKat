#pragma once

#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class ResourceSystem;
}

namespace CKE {
	// TODO: Convert into a lightweight i32 in release
	using PipelineID = String;

	struct PipelineIDS
	{
		inline static const PipelineID PassThrough{"PassThrough"};
		inline static const PipelineID DepthPrePass{"DepthPrePass"};
		inline static const PipelineID BloomUpscale{"BloomUpscale"};
		inline static const PipelineID GBufferPass{"GBufferPass"};
		inline static const PipelineID IntensityCheckPass{"IntensityCheckPass"};
	};

	// Automatically manages creating and retrieving render pipelines.
	// Uses a cache of already created pipelines to avoid duplicates
	// TODO: Cache layouts
	class PipelineManager
	{
	public:
		void           Initialize(RenderDevice* pDevice, ResourceSystem* pResources);
		void           CreateFromAsset(PipelineID idToAssign, Path assetPath, GraphicsPipelineDesc desc);
		PipelineHandle GetPipeline(PipelineID id);

	private:
		struct CachedPipelineInfo
		{
			PipelineHandle      m_Handle; // Handle in the RenderAPI
			PipelineID          m_ID;     // ID in the manager
			Path                m_Path;   // Path on resources
			GraphicsPipelineDesc m_Desc;
		};

		ResourceSystem*                     m_pResources{nullptr};
		RenderDevice*                       m_pDevice{nullptr};
		Map<PipelineID, CachedPipelineInfo> m_Cache;
	};
}
