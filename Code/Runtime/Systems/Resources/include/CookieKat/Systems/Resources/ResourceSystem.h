#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Threading/Threading.h"
#include "CookieKat/Core/Memory/PoolAllocator.h"

#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"
#include "CookieKat/Systems/Resources/ResourceID.h"
#include "CookieKat/Systems/Resources/IResource.h"
#include "CookieKat/Systems/Resources/ResourceTypeID.h"
#include "CookieKat/Systems/Resources/ResourceLoader.h"
#include "CookieKat/Systems/Resources/ResourceRecord.h"
#include "CookieKat/Systems/TaskSystem/TaskSystem.h"

namespace CKE {
	struct ResourceSystemSettings
	{
		u32  m_MaxLoadedResources = 25'000;
		Path m_BaseDataPath = "../../../../Data/";
	};

	struct AsyncInstallRequestState
	{
		ResourceID          m_ResourceID{};
		IResource*          m_pResource = nullptr;
		InstallDependencies m_InstallDependencies{};
		ResourceLoader*     m_pLoader = nullptr;
	};

	class ResourceStreamingJob : ITaskSet
	{
	public:
		ResourceStreamingJob() : ITaskSet{1} {
			m_PendingLoad.reserve(250);
			m_Loaded.reserve(250);
		}

		void ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) override;

	private:
		friend class ResourceSystem;
		ResourceSystemSettings m_Settings{};

		Threading::Mutex m_AccessMutex; // Use to sync the access to data below

		Vector<AsyncLoadRequestState*> m_PendingLoad; // Request still pending to load 
		Vector<AsyncLoadRequestState*> m_Loaded;      // Request that have been loaded from disk
	};
}

namespace CKE {
	class ResourceSystem : public IEngineSystem
	{
	public:
		//-----------------------------------------------------------------------------
		// Lifetime
		//-----------------------------------------------------------------------------

		// These methods must be called following this structure:
		//   Initialize();
		//   while(running) { UpdateStreaming(); }
		//   Shutdown();

		void Initialize(TaskSystem* pTaskSystem);
		void UpdateStreaming(); // Called on the main thread
		void Shutdown();

		//-----------------------------------------------------------------------------
		// Resource Management
		//-----------------------------------------------------------------------------

		// Typeless API
		//-----------------------------------------------------------------------------

		// Records an async request to load a resource.
		// Loading status can be checked with IsResourceLoaded(...)
		ResourceID LoadResourceAsync(Path resourcePath);

		// Checks if the resource is ready to be used
		inline bool IsResourceLoaded(ResourceID id);

		// Loads a resource synchronously, blocking the calling thread
		// If you don't want this block, use LoadResourceAsync(...)
		ResourceID LoadResource(Path resourcePath);

		// Returns a pointer the underlying resource data
		//
		// Asserts:
		//   ResourceID is valid and registered to a resource
		//   Resource is ready to use
		//   Resource is not null
		//
		IResource* GetResource(ResourceID resourceID);

		// Unloads a resource synchronously, blocking the calling thread
		// TODO: Still in development, this doesn't do anything
		void UnloadResource(ResourceID resourceID);

		// Template API
		//-----------------------------------------------------------------------------

		// For all of these template methods, check the non-template version for a description.

		template <typename T>
			requires std::is_base_of_v<IResource, T>
		TResourceID<T> LoadResourceAsync(Path resourcePath);

		template <typename T>
		bool IsResourceLoaded(TResourceID<T> id);

		template <typename T>
			requires std::is_base_of_v<IResource, T>
		TResourceID<T> LoadResource(Path resourcePath);

		template <typename T>
			requires std::is_base_of_v<IResource, T>
		T* GetResource(ResourceID resourceID);

		//-----------------------------------------------------------------------------
		// Resource Loaders
		//-----------------------------------------------------------------------------

		// The ptr to the loader must remain valid until its unregistered
		void RegisterLoader(ResourceLoader* pResourceLoader);
		void UnRegisterLoader(ResourceLoader* pResourceLoader);

	private:
		// Returns the resource loader for a given resource
		void GetResourceLoader(Path resourcePath, ResourceLoader*& pLoader);

		// Returns the next available resource ID and marks it as in-use
		ResourceID GetNextResourceID();

	private:
		// References to external systems
		//-----------------------------------------------------------------------------

		TaskSystem* m_pTaskSystem = nullptr;

		// Resources Database
		//-----------------------------------------------------------------------------

		ResourceSystemSettings               m_Settings;
		Map<ResourceTypeID, ResourceLoader*> m_pResourceLoaders;
		Map<ResourceID, ResourceRecord*>     m_ResourceRecords;
		Map<Path, ResourceID>                m_PathToResourceID;

		Queue<ResourceID> m_AvailableResourceIDs{};

		// Streaming Process Data
		//-----------------------------------------------------------------------------

		TPoolAllocator<PendingLoadRequest>       m_PendingLoadRequestAllocator;
		TPoolAllocator<AsyncInstallRequestState> m_AsyncInstallRequestAllocator;
		TPoolAllocator<AsyncLoadRequestState>    m_AsyncRequestStateAllocator;
		TPoolAllocator<ResourceRecord>           m_ResourceRecordAllocator;

		Vector<PendingLoadRequest*>          m_PendingLoadRequestsToSubmit; // Requests waiting to be submitted to the streaming thread
		Map<ResourceID, PendingLoadRequest*> m_InProgressRequests; // Pretty much only async load requests
		Vector<PendingLoadRequest*>          m_WaitingInstallRequests; // Requests waiting installation until dependencies are loaded
		Vector<PendingLoadRequest*>          m_RequestsToInstall; // Requests that will be installed this update
		Vector<PendingLoadRequest*>          m_StillWaitingRequests; // Requests still waiting dependencies to install
		ResourceStreamingJob                 m_ResourceStreamingJob;
	};
}

namespace CKE {
	// Template Definitions
	//-----------------------------------------------------------------------------

	template <typename T>
		requires std::is_base_of_v<IResource, T>
	T* ResourceSystem::GetResource(ResourceID resourceID) {
		return static_cast<T*>(m_ResourceRecords[resourceID]->m_pResource);
	}

	template <typename T>
	bool ResourceSystem::IsResourceLoaded(TResourceID<T> id) {
		return m_ResourceRecords[ResourceID{id.m_Value}]->m_IsReadyToUse;
	}

	template <typename T>
		requires std::is_base_of_v<IResource, T>
	TResourceID<T> ResourceSystem::LoadResourceAsync(Path resourcePath) {
		return TResourceID<T>{LoadResourceAsync(resourcePath)};
	}

	template <typename T>
		requires std::is_base_of_v<IResource, T>
	TResourceID<T> ResourceSystem::LoadResource(Path resourcePath) {
		static_assert(std::is_base_of_v<IResource, T>);
		return TResourceID<T>{LoadResource(resourcePath)};
	}

	bool ResourceSystem::IsResourceLoaded(ResourceID id) {
		return m_ResourceRecords[id]->m_IsReadyToUse;
	}
}
