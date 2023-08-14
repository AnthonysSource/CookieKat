#pragma once

#include "CookieKat/Core/FileSystem/FileSystem.h"

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include "CookieKat/Systems/Resources/ResourceLoader.h"
#include "CookieKat/Systems/RenderUtils/TextureUploader.h"

namespace CKE {
	class TextureLoader : public CompiledResourcesLoader
	{
	public:
		void Initialize(RenderDevice* pRenderDevice);
		void Shutdown();

		LoadResult LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) override;
		LoadResult Install(InstallContext& ctx) override;
		LoadResult Uninstall(UninstallContext& ctx) override;
		LoadResult Unload(UnloadContext& ctx) override;

		Array<ResourceTypeID, 16> GetLoadableTypes() override { return {ResourceTypeID("tex")}; }

	private:
		RenderDevice* m_pRenderDevice = nullptr;
		TextureUploader m_Uploader{};
	};

	class CubeMapLoader : public CompiledResourcesLoader
	{
	public:
		void Initialize(RenderDevice* pRenderDevice);

		LoadResult LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) override;
		LoadResult Install(InstallContext& ctx) override;
		LoadResult Uninstall(UninstallContext& ctx) override;
		LoadResult Unload(UnloadContext& ctx) override;

		Array<ResourceTypeID, 16> GetLoadableTypes() override { return {ResourceTypeID("cubeMap")}; }

	private:
		RenderDevice* m_pRenderDevice = nullptr;
	};
}
