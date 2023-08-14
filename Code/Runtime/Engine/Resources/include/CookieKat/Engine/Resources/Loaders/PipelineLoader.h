#pragma once

#include "CookieKat/Systems/Resources/ResourceLoader.h"

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class PipelineLoader : public CompiledResourcesLoader
	{
	public:
		void Initialize(RenderDevice* pRenderDevice);

		LoadResult Install(InstallContext& ctx) override;
		LoadResult Uninstall(UninstallContext& ctx) override;
		LoadResult Unload(UnloadContext& ctx) override;

		LoadResult LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) override;

		Array<ResourceTypeID, 16> GetLoadableTypes() override { return{ ResourceTypeID("pipeline") }; }

	private:
		RenderDevice* m_pRenderDevice = nullptr;
	};
}
