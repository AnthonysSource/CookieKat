#include "ResourceSystem.h"

#include "CookieKat/Core/Platform/PlatformTime.h"
#include "CookieKat/Core/Logging/LoggingSystem.h"
#include "CookieKat/Core/Memory/Memory.h"

#include <chrono>

#include "CookieKat/Core/Profilling/Profilling.h"

namespace CKE {
	void ResourceStreamingJob::ExecuteRange(enki::TaskSetPartition range_, uint32_t threadnum_) {
		CKE_PROFILE_EVENT()
		// We don't have to lock these accesses because the resource system doesn't use the data
		// while the job is running
		for (i32 i = m_PendingLoad.size() - 1; i >= 0; --i) {
			AsyncLoadRequestState* r = m_PendingLoad[i];
			String const           fullPath = m_Settings.m_BaseDataPath + r->m_Path;
			r->m_RawData = g_FileSystem.ReadBinaryFile(fullPath);

			LoadContext loadContext{&r->m_RawData, r->m_ResourceID, r->m_Path};
			if (r->m_pLoader->Load(loadContext, r->m_LoadOutput) == LoadResult::Failed) {
				g_LoggingSystem.Log(LogLevel::Error, LogChannel::Resources, "Resource loading failed successfully! {}\n", r->m_Path);
			}
			CKE_ASSERT(r->m_LoadOutput.m_pResource != nullptr);

			m_Loaded.push_back(r);
			m_PendingLoad.pop_back();
		}
	}

	void ResourceSystem::Initialize(TaskSystem* pTaskSystem) {
		CKE_PROFILE_EVENT()
		CKE_ASSERT(pTaskSystem != nullptr);
		// Save required references
		m_pTaskSystem = pTaskSystem;

		// Generate possible runtime resource IDs
		for (u64 i = 1; i <= m_Settings.m_MaxLoadedResources; ++i) {
			m_AvailableResourceIDs.push(ResourceID{i});
		}

		m_ResourceRecords.reserve(m_Settings.m_MaxLoadedResources);
		m_PathToResourceID.reserve(m_Settings.m_MaxLoadedResources);
		m_pResourceLoaders.reserve(64);

		// Allocators
		m_ResourceRecordAllocator = TPoolAllocator<ResourceRecord>{
			Memory::Alloc(sizeof(ResourceRecord) * m_Settings.m_MaxLoadedResources), m_Settings.m_MaxLoadedResources
		};
		m_PendingLoadRequestAllocator = TPoolAllocator<PendingLoadRequest>{ Memory::Alloc(sizeof(PendingLoadRequest) * 200), 200};
		m_AsyncRequestStateAllocator = TPoolAllocator<AsyncLoadRequestState>{ Memory::Alloc(sizeof(AsyncLoadRequestState) * 200), 200};
		m_AsyncInstallRequestAllocator = TPoolAllocator<AsyncInstallRequestState>{ Memory::Alloc(sizeof(AsyncInstallRequestState) * 200), 200};
	}

	void ResourceSystem::UpdateStreaming() {
		CKE_PROFILE_EVENT()

		// We don't do anything if there is a streaming job currently in progress
		if (!m_ResourceStreamingJob.GetIsComplete()) { return; }

		{
			Threading::Lock scopedLock(m_ResourceStreamingJob.m_AccessMutex);

			// Submit new pending request to streaming thread
			//-----------------------------------------------------------------------------
			for (PendingLoadRequest* pendingRequest : m_PendingLoadRequestsToSubmit) {
				// Find the request state of the pending request if it exists
				AsyncLoadRequestState* loadRequest = nullptr;
				for (AsyncLoadRequestState* request : m_ResourceStreamingJob.m_PendingLoad) {
					if (request->m_ResourceID == pendingRequest->m_ResourceID) {
						loadRequest = request;
						break;
					}
				}

				if (loadRequest == nullptr) {
					// Create load request
					loadRequest = m_AsyncRequestStateAllocator.New();
					loadRequest->m_Path = pendingRequest->m_Path;
					loadRequest->m_ResourceID = pendingRequest->m_ResourceID;
					GetResourceLoader(loadRequest->m_Path, loadRequest->m_pLoader);

					m_ResourceStreamingJob.m_PendingLoad.emplace_back(loadRequest);
				}

				m_InProgressRequests.insert({pendingRequest->m_ResourceID, pendingRequest});
			}
			m_PendingLoadRequestsToSubmit.clear();

			// Process already loaded requests
			//-----------------------------------------------------------------------------
			for (AsyncLoadRequestState* r : m_ResourceStreamingJob.m_Loaded) {
				g_LoggingSystem.Log(LogLevel::Info, LogChannel::Resources, "Request Loaded: {}\n",
				                    r->m_Path);

				ResourceRecord* pRecord = m_ResourceRecords[r->m_ResourceID];
				pRecord->m_pResource = r->m_LoadOutput.m_pResource;

				// Get install dependencies and add resource dependency links
				InstallDependencies installDependencies{};
				for (Path const& dependencyPath : r->m_LoadOutput.m_Dependencies) {
					ResourceID dependencyID = LoadResourceAsync(dependencyPath);
					pRecord->m_Dependencies.push_back(dependencyID);

					// Add user to child resource
					ResourceRecord* pDependencyRecord = m_ResourceRecords[dependencyID];
					pDependencyRecord->m_Users.push_back(dependencyID);

					// Save install dependencies 
					installDependencies.m_DependencyIDs.emplace_back(dependencyID);
				}

				// Update load request and transfer it to the next stage
				PendingLoadRequest* loadRequest = m_InProgressRequests[r->m_ResourceID];
				loadRequest->m_Deps = installDependencies;
				m_WaitingInstallRequests.push_back(loadRequest);
				m_InProgressRequests.erase(r->m_ResourceID);

				m_AsyncRequestStateAllocator.Delete(r);
			}
			m_ResourceStreamingJob.m_Loaded.clear();

			// Update what resources can be installed
			//-----------------------------------------------------------------------------
			for (PendingLoadRequest* r : m_WaitingInstallRequests) {
				bool isReadyToInstall = true;
				for (ResourceID depID : r->m_pRecord->m_Dependencies) {
					if (!m_ResourceRecords[depID]->m_IsReadyToUse) {
						isReadyToInstall = false;
						break;
					}
				}

				if (isReadyToInstall) { m_RequestsToInstall.push_back(r); }
				else { m_StillWaitingRequests.push_back(r); }
			}
			m_WaitingInstallRequests = m_StillWaitingRequests;
			m_StillWaitingRequests.clear();

			// Install requests
			//-----------------------------------------------------------------------------
			for (PendingLoadRequest* r : m_RequestsToInstall) {
				ResourceRecord* pRecord = r->m_pRecord;
				ResourceLoader* pLoader = nullptr;
				GetResourceLoader(r->m_Path, pLoader);

				InstallContext installContext{pRecord->m_pResource, r->m_Deps};
				if (pLoader->Install(installContext) != LoadResult::Successful) {
					g_LoggingSystem.Log(LogLevel::Error, LogChannel::Assets, "Failed Install: {}", r->m_Path);
				}

				pRecord->m_IsReadyToUse = true;
				g_LoggingSystem.Log(LogLevel::Info, LogChannel::Resources, "Request Installed: {}\n",
				                    pRecord->m_Path);

				m_PendingLoadRequestAllocator.Delete(r);
			}
			m_RequestsToInstall.clear();
		}

		//-----------------------------------------------------------------------------

		// If the job has pending data to process, we submit it to the job system
		if (!m_ResourceStreamingJob.m_PendingLoad.empty()) {
			m_pTaskSystem->ScheduleTask(&m_ResourceStreamingJob);
		}
	}

	void ResourceSystem::Shutdown() {
		// Release all of the allocators
		Memory::Free(m_AsyncRequestStateAllocator.GetUnderlyingMemoryBuffer());
		Memory::Free(m_PendingLoadRequestAllocator.GetUnderlyingMemoryBuffer());
		Memory::Free(m_ResourceRecordAllocator.GetUnderlyingMemoryBuffer());
		Memory::Free(m_AsyncInstallRequestAllocator.GetUnderlyingMemoryBuffer());
	}

	//-----------------------------------------------------------------------------

	ResourceID ResourceSystem::LoadResourceAsync(Path resourcePath) {
		CKE_PROFILE_EVENT();
		ResourceID resourceID;

		// Get existing resource ID or create a new one
		if (m_PathToResourceID.contains(resourcePath)) {
			resourceID = m_PathToResourceID[resourcePath];
		}
		else {
			resourceID = GetNextResourceID();

			ResourceRecord* pRecord = m_ResourceRecordAllocator.New();
			pRecord->m_Path = resourcePath;
			pRecord->m_ID = resourceID;
			pRecord->m_IsReadyToUse = false;

			m_PathToResourceID.insert({resourcePath, resourceID});
			m_ResourceRecords.insert({resourceID, pRecord});
		}

		// Add request to pending list
		PendingLoadRequest* pPendingRequest = m_PendingLoadRequestAllocator.New();
		pPendingRequest->m_Path = resourcePath;
		pPendingRequest->m_ResourceID = resourceID;
		pPendingRequest->m_pRecord = m_ResourceRecords[resourceID];
		m_PendingLoadRequestsToSubmit.emplace_back(pPendingRequest);

		// Return ID so the user can query resource loading status
		return resourceID;
	}

	void ResourceSystem::GetResourceLoader(Path resourcePath, ResourceLoader*& pLoader) {
		u64                  i = resourcePath.find_last_of('.');
		String               extension = resourcePath.substr(i + 1);
		const ResourceTypeID resTypeID{extension};

		auto const loaderPair = m_pResourceLoaders.find(resTypeID);
		if (loaderPair == m_pResourceLoaders.end()) {
			g_LoggingSystem.Log(LogLevel::Fatal, LogChannel::Assets, "Couldn't find a loader for the given extension [{}]", extension);
		}
		pLoader = loaderPair->second;
	}

	ResourceID ResourceSystem::GetNextResourceID() {
		CKE_ASSERT(!m_AvailableResourceIDs.empty()); // Check we didn't run out of IDs
		ResourceID id = m_AvailableResourceIDs.front();
		m_AvailableResourceIDs.pop();
		return id;
	}

	ResourceID ResourceSystem::LoadResource(Path resourcePath) {
		CKE_PROFILE_EVENT();
		auto startTime = std::chrono::system_clock::now();

		// Check if its already loaded and return if so
		//-----------------------------------------------------------------------------

		if (m_PathToResourceID.contains(resourcePath)) {
			return m_PathToResourceID[resourcePath];
		}

		// Create a record
		//-----------------------------------------------------------------------------

		ResourceID id = GetNextResourceID();

		ResourceRecord* pRecord = m_ResourceRecordAllocator.New();
		pRecord->m_Path = resourcePath;
		pRecord->m_ID = id;
		pRecord->m_IsReadyToUse = true;

		m_PathToResourceID.insert({resourcePath, id});
		m_ResourceRecords.insert({id, pRecord});

		// Load binary data
		//-----------------------------------------------------------------------------

		String const fullPath = m_Settings.m_BaseDataPath + resourcePath;
		Blob         blob = g_FileSystem.ReadBinaryFile(fullPath);

		// Get file extension from path and search the loader for the given type
		//-----------------------------------------------------------------------------

		ResourceLoader* pLoader = nullptr;
		GetResourceLoader(resourcePath, pLoader);
		CKE_ASSERT(pLoader != nullptr); // We haven't found a loader for the given resource type

		// Load Resource and dependencies
		//-----------------------------------------------------------------------------

		LoadContext loadContext{&blob, pRecord->m_ID, pRecord->m_Path};
		LoadOutput  loadOutput{};
		if (pLoader->Load(loadContext, loadOutput) == LoadResult::Failed) {
			g_LoggingSystem.Log(LogLevel::Fatal, LogChannel::Assets, "Load failed -> AssetPath: {}", resourcePath);
		}
		CKE_ASSERT(loadOutput.m_pResource != nullptr);
		pRecord->m_pResource = loadOutput.m_pResource;

		// Load and install dependencies
		InstallDependencies installDependencies{};
		for (Path const& dependencyPath : loadOutput.m_Dependencies) {
			ResourceID dependencyID = LoadResource(dependencyPath);
			pRecord->m_Dependencies.push_back(dependencyID);

			// Add user to child resource
			ResourceRecord* pDependencyRecord = m_ResourceRecords[dependencyID];
			pDependencyRecord->m_Users.push_back(dependencyID);

			// Save install dependencies 
			installDependencies.m_DependencyIDs.emplace_back(dependencyID);
		}

		// Install parent resource
		InstallContext installContext{pRecord->m_pResource, installDependencies};
		if (pLoader->Install(installContext) == LoadResult::Failed) {
			g_LoggingSystem.Log(LogLevel::Fatal, LogChannel::Assets, "Install failed -> AssetPath: {}", resourcePath);
		}

		// Time to load tracking
		auto endTime = std::chrono::system_clock::now();
		auto elapsed =
				std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
		g_LoggingSystem.Log(LogLevel::Info, LogChannel::Assets, "Loaded {} / Time: {}ms\n", pRecord->m_Path,
		                    elapsed.count());

		return pRecord->m_ID;
	}

	void ResourceSystem::UnloadResource(ResourceID resourceID) {
		//CKE_ASSERT(m_pResourceDatabase.contains(resourceID));

		//ResourceRecord& record = m_pResourceDatabase[resourceID];
		//if (!record.m_Users.empty()) {
		//	g_LoggingSystem.Log(LogLevel::Fatal, LogChannel::Assets, "Trying to unload a resource with users");
		//	return;
		//}

		//// Get path from ID and remove the map entry
		//Path path{};
		//for (auto& [p, id] : m_PathToResourceID) {
		//	if (id == resourceID) {
		//		m_PathToResourceID.erase(p);
		//		path = p;
		//		break;
		//	}
		//}

		//// Get Loader
		//ResourceLoader* pLoader = nullptr;
		//GetResourceLoader(path, pLoader);
		//CKE_ASSERT(pLoader != nullptr); // We haven't found a loader for the given resource type

		//pLoader->Uninstall(record);
		//CKE_ASSERT(record.GetResource() != nullptr);

		//for (Path& dependencyPath : record.m_Dependencies) {
		//	ResourceID      dependencyID = LoadResource(dependencyPath);
		//	ResourceRecord& dependencyRecord = m_pResourceDatabase[dependencyID];

		//	// Remove this user from the dependency
		//	Vector<Path>& deps = dependencyRecord.m_Users;
		//	for (int i = 0; i < deps.size(); ++i) {
		//		if (deps[i] == path) {
		//			deps[i] = deps.back();
		//			deps.pop_back();
		//		}
		//	}

		//	// If there are no more users of the dependency, unload it
		//	if (deps.empty()) {
		//		UnloadResource(dependencyID);
		//	}
		//}

		//// Unload parent resource
		//pLoader->Unload(record);

		//// Return the ID as available
		//m_AvailableResourceIDs.push(resourceID);
		//resourceID.m_ID = 0;
	}

	IResource* ResourceSystem::GetResource(ResourceID resourceID) {
		CKE_ASSERT(m_ResourceRecords.contains(resourceID));
		ResourceRecord const* pRecord = m_ResourceRecords[resourceID];
		CKE_ASSERT(pRecord->m_pResource != nullptr);
		CKE_ASSERT(pRecord->m_IsReadyToUse);
		return pRecord->m_pResource;
	}

	//-----------------------------------------------------------------------------

	void ResourceSystem::RegisterLoader(ResourceLoader* pResourceLoader) {
		for (ResourceTypeID resTypeID : pResourceLoader->GetLoadableTypes()) {
			if (resTypeID == ResourceTypeID{}) { break; }
			// Check that there are not already loaders registered for a given type
			CKE_ASSERT(!m_pResourceLoaders.contains(resTypeID));
			m_pResourceLoaders.insert({resTypeID, pResourceLoader});
		}
	}

	void ResourceSystem::UnRegisterLoader(ResourceLoader* pResourceLoader) {
		for (ResourceTypeID resTypeID : pResourceLoader->GetLoadableTypes()) {
			if (m_pResourceLoaders.contains(resTypeID)) {
				// TODO: replace ptr comparisong with loader type ID
				if (m_pResourceLoaders.at(resTypeID) == pResourceLoader) {
					m_pResourceLoaders.erase(resTypeID);
				}
			}
		}
	}
}
