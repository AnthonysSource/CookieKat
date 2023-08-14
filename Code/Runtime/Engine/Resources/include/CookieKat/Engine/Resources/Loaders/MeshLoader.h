#pragma once

#include "CookieKat/Systems/Resources/ResourceLoader.h"

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	class MeshLoader : public ResourceLoader
	{
	public:
		void Initialize(RenderDevice* pRenderDevice);

		LoadResult Load(LoadContext& ctx, LoadOutput& out) override;
		LoadResult Install(InstallContext& ctx) override;
		LoadResult Unload(UnloadContext& ctx) override;

		Array<ResourceTypeID, 16> GetLoadableTypes() override { return {ResourceTypeID("fbx"), ResourceTypeID("obj")}; }

	private:
		RenderDevice* m_pDevice = nullptr;
	};
}
