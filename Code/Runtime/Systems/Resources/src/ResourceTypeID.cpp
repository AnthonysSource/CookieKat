#include "ResourceTypeID.h"

#include <memory>
#include "CookieKat/Core/Containers/String.h"

namespace CKE
{
	ResourceTypeID::ResourceTypeID(String const& extension)
	{
		m_ID = std::hash<String>()(extension);
	}
}
