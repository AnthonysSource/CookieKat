#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"

#include "CookieKat/Systems/Resources/ResourceID.h"
#include "CookieKat/Systems/Resources/IResource.h"
#include "CookieKat/Systems/Resources/InstallDependencies.h"

namespace CKE {
	// Wrapper for a list of resources that need to be installed before
	// a specific resource is installed.
	//
	// This list is populated by the resource system and is passed to the
	// public loader when the dependencies have already been installed.
	// The user can acquire these dependencies by calling the class methods
	class InstallDependencies
	{
		friend class ResourceSystem;

	public:
		template <typename T>
			requires std::is_base_of_v<IResource, T>
		TResourceID<T> GetInstallDependency(u64 dependencyIndexInResourceHeader) const {
			TResourceID<T> r{};
			r.m_Value = m_DependencyIDs[dependencyIndexInResourceHeader].m_Value;
			return r;
		}

	private:
		Vector<ResourceID> m_DependencyIDs;
	};
}

namespace CKE {
	enum class LoadResult : bool
	{
		Successful = true,
		Failed = false
	};

	enum class InstallResult
	{
		Completed,
		Failed,
		InProgress,
	};

	class LoadOutput
	{
	public:
		inline void SetResource(IResource* pResource) {
			CKE_ASSERT(pResource != nullptr);
			m_pResource = pResource;
		}

		inline void AddDependency(Path const& path) {
			m_Dependencies.emplace_back(path);
		}

	private:
		friend ResourceSystem;
		friend class ResourceStreamingJob;

		IResource*   m_pResource = nullptr; // Ptr to the resource allocated by the loader
		Vector<Path> m_Dependencies;        // Required dependencies for the loaded resource
	};

	class LoadContext
	{
	public:
		LoadContext(Vector<u8>* mBinaryData, ResourceID mId, Path mAssetPath)
			: m_BinaryData{mBinaryData},
			  m_ID{mId},
			  m_AssetPath{mAssetPath} {}

		inline ResourceID  GetResourceID() const { return m_ID; }
		inline Path const& GetPath() const { return m_AssetPath; }

		inline Vector<u8> const& GetData() const {
			CKE_ASSERT(m_BinaryData != nullptr);
			return *m_BinaryData;
		}

	private:
		friend ResourceSystem;
		Vector<u8>* m_BinaryData; // Pointer to the binary data of the asset;
		ResourceID  m_ID;         // Runtime identifier in the database
		Path        m_AssetPath;  // Unique Asset identifier and path of the resource in the file system
	};

	class InstallContext
	{
	public:
		InstallContext(IResource* mPResource, InstallDependencies mDependencies)
			: m_pResource{mPResource},
			  m_Dependencies{mDependencies} {}

		template <typename T>
		inline T* GetResource() {
			CKE_ASSERT(m_pResource != nullptr);
			return reinterpret_cast<T*>(m_pResource);
		}

		inline InstallDependencies& GetInstallDependencies() {
			return m_Dependencies;
		}

	private:
		friend ResourceSystem;
		IResource*          m_pResource = nullptr; // Ptr to the resource allocated by the loader
		InstallDependencies m_Dependencies;
	};

	class UninstallContext
	{
	public:
		template <typename T>
		inline T* GetResource() {
			CKE_ASSERT(m_pResource != nullptr);
			return reinterpret_cast<T*>(m_pResource);
		}

	private:
		IResource* m_pResource = nullptr; // Ptr to the resource allocated by the loader

		friend ResourceSystem;
	};

	class UnloadContext
	{
	public:
		template <typename T>
		inline T* GetResource() {
			CKE_ASSERT(m_pResource != nullptr);
			return reinterpret_cast<T*>(m_pResource);
		}

	private:
		friend ResourceSystem;
		IResource* m_pResource = nullptr; // Ptr to the resource allocated by the loader
	};

	//-----------------------------------------------------------------------------

	// Base Interface for all of the loaders in the resource system
	class ResourceLoader
	{
	public:
		//-----------------------------------------------------------------------------

		// Handles loading the resource
		virtual LoadResult Load(LoadContext& ctx, LoadOutput& out) = 0;

		// Allows Post-Loading logic for the resource if necessary
		virtual LoadResult Install(InstallContext& ctx) { return LoadResult::Successful; }

		// Allows Pre-Unloading logic for the resource if necessary
		virtual LoadResult Uninstall(UninstallContext& ctx) { return LoadResult::Successful; }

		// Handles unloading the resource
		virtual LoadResult Unload(UnloadContext& ctx) = 0;

		//-----------------------------------------------------------------------------

		// This method is useful for defining async Install(...) procedures.
		virtual InstallResult CheckInstallStatus() { return InstallResult::Completed; }

		// Returns the resource types that the loader will handle
		virtual Array<ResourceTypeID, 16> GetLoadableTypes() = 0;

		//-----------------------------------------------------------------------------

		virtual ~ResourceLoader() = default;
	};

	class CompiledResourcesLoader : public ResourceLoader
	{
	public:
		virtual LoadResult LoadCompiledResource(LoadContext& ctx, BinaryInputArchive& ar, LoadOutput& out) = 0;

	private:
		virtual LoadResult Load(LoadContext& ctx, LoadOutput& out) override;
	};
}

namespace CKE {
	inline LoadResult CompiledResourcesLoader::Load(LoadContext& ctx, LoadOutput& out) {
		BinaryInputArchive ar;
		ar.ReadFromBlob(ctx.GetData());

		ResourceHeader header;
		ar << header;

		for (Path& dependency : header.m_DependencyPaths) {
			out.AddDependency(dependency);
		}

		return LoadCompiledResource(ctx, ar, out);
	}
}
