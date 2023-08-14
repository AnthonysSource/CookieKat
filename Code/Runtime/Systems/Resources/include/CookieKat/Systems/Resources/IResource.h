#pragma once

#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Serialization/Archive.h"

namespace CKE {
	// Header that is added to all of the compiled resources
	struct ResourceHeader
	{
		CKE_SERIALIZE(m_ResourceType, m_ResourcePath, m_DependencyPaths);

		u32          m_ResourceType{};
		Path         m_ResourcePath;
		Vector<Path> m_DependencyPaths;
	};

	// Base resource interface for type-safety
	class IResource { };
}
