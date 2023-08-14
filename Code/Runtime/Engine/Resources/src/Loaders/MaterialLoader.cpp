#include "Loaders/MaterialLoader.h"

#include "CookieKat/Core/Memory/Memory.h"
#include "CookieKat/Systems/Resources/InstallDependencies.h"
#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"

namespace CKE {
	LoadResult MaterialLoader::LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) {
		RenderMaterialResource* material = New<RenderMaterialResource>();
		ar << *material;
		out.SetResource(material);
		return LoadResult::Successful;
	}

	LoadResult MaterialLoader::Install(InstallContext& ctx) {
		auto material = ctx.GetResource<RenderMaterialResource>();

		InstallDependencies dep = ctx.GetInstallDependencies();
		material->m_AlbedoTexture = dep.GetInstallDependency<RenderTextureResource>(material->m_AlbedoTexture.m_Value);
		material->m_RoughnessTexture = dep.GetInstallDependency<RenderTextureResource>(material->m_RoughnessTexture.m_Value);
		material->m_MetalicTexture = dep.GetInstallDependency<RenderTextureResource>(material->m_MetalicTexture.m_Value);
		material->m_NormalTexture = dep.GetInstallDependency<RenderTextureResource>(material->m_NormalTexture.m_Value);

		return LoadResult::Successful;
	}

	LoadResult MaterialLoader::Unload(UnloadContext& ctx) {
		auto mat = ctx.GetResource<RenderMaterialResource>();
		CKE::Delete(mat);
		return LoadResult::Successful;
	}
}
