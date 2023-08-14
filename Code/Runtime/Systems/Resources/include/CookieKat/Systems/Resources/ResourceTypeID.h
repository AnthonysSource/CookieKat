#pragma once

#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Serialization/Archive.h"

namespace CKE {
	// Unique u64 ID generated from the extension of the file
	class ResourceTypeID
	{
		CKE_SERIALIZE(m_ID);

	public:
		ResourceTypeID() = default;
		ResourceTypeID(String const& extension);

		u64 GetID() const { return m_ID; }

		bool operator==(ResourceTypeID const& other) const { return m_ID == other.m_ID; }

	private:
		u64 m_ID{};
	};
}

// Hash Code
//-----------------------------------------------------------------------------
namespace std {
	template <>
	struct hash<CKE::ResourceTypeID>
	{
		std::size_t operator()(CKE::ResourceTypeID const& k) const noexcept {
			return std::hash<CKE::u64>()(k.GetID());
		}
	};
}
