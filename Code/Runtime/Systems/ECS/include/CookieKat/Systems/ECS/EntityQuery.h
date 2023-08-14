#pragma once

#include "IDs.h"

namespace CKE {
	// TODO: Still in development
	// Data structures for an advanced component querying method

	enum class QueryOp
	{
		And,
		Or,
		Not,
		Optional,
	};

	enum class QueryAccess
	{
		ReadWrite,
		ReadOnly,
		WriteOnly
	};

	struct QueryElement
	{
		ComponentTypeID m_ComponentID;
		QueryAccess     m_Access = QueryAccess::ReadWrite;
		QueryOp         m_Op = QueryOp::And;
	};

	struct QueryInfo
	{
		Array<QueryElement, 16> m_QueryElements;
		u32                     m_QueryElementsCount;
	};
}

namespace CKE {
}
