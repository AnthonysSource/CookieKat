#pragma once

#include "CookieKat/Core/Containers/Containers.h"

#include "CookieKat/Systems/Resources/ResourceID.h"
#include "CookieKat/Systems/Resources/IResource.h"

namespace CKE {
	struct ResourceRecord
	{
		ResourceID         m_ID;                  // Runtime identifier in the database
		Path               m_Path;                // Unique identifier and resource relative path in the file system
		IResource*         m_pResource = nullptr; // Ptr to the resource
		Vector<ResourceID> m_Dependencies;        // Resources that *are used* by this resource
		Vector<ResourceID> m_Users;               // Resources that *use* this resource
		bool               m_IsReadyToUse = false;
	};

	class RequesterInfo
	{
	public:
		RequesterInfo() = default;

		RequesterInfo(u32 id) : m_ID{id} {
			m_Type = Type::Identifiable;
		}

		enum class Type
		{
			Manual,
			Identifiable,
		};

		u32  m_ID = 0;
		Type m_Type = Type::Manual;
		bool m_IsInstallDependency = false;
	};

	struct PendingLoadRequest
	{
		Path                m_Path;          // Path of the resource
		ResourceID          m_ResourceID;    // ID assigned to the resource
		RequesterInfo       m_RequesterInfo; // Who requested the resource
		ResourceRecord*     m_pRecord;
		InstallDependencies m_Deps;
	};

	// State of an In-Progress Request
	struct AsyncLoadRequestState
	{
		Path            m_Path;
		ResourceID      m_ResourceID;
		Vector<u8>      m_RawData{};
		ResourceLoader* m_pLoader = nullptr;
		LoadOutput      m_LoadOutput;
	};
}
