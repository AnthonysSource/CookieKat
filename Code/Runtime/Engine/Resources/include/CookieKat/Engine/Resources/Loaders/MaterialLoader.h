#pragma once

#include "CookieKat/Systems/Resources/ResourceLoader.h"

namespace CKE {
	class MaterialLoader : public CompiledResourcesLoader
	{
	public:
		LoadResult LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) override;
		LoadResult Install(InstallContext& ctx) override;
		LoadResult Unload(UnloadContext& ctx) override;

		Array<ResourceTypeID, 16> GetLoadableTypes() override { return {ResourceTypeID("mat")}; }
	};
}
