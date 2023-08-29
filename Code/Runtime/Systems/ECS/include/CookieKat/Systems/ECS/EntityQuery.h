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
		ComponentTypeID m_ComponentTypeID;
		QueryAccess     m_Access = QueryAccess::ReadWrite;
		QueryOp         m_Op = QueryOp::And;
	};

	struct QueryInfo
	{
		Array<QueryElement, 16> m_QueryElements;
		u32                     m_QueryElementsCount;
	};

	struct ArchetypeQueryResult
	{
		ArchetypeID                      m_ArchetypeID;
		u64                              m_TotalRows;
		Vector<ArchetypeComponentColumn> m_ComponentColumns; // Component order is the same as the query order
	};

	struct QueryResult
	{
		Vector<ArchetypeQueryResult> m_MatchingArchetypes;
		u64 m_TotalEntities;
	};
}

namespace CKE {
	class QueryBuilder
	{
	public:
		template <typename T>
		QueryBuilder& Add();

		QueryInfo Build();

	private:
		QueryInfo m_QueryInfo{};
		u32 m_Index = 0;
	};

	template <typename T>
	QueryBuilder& QueryBuilder::Add() {
		m_QueryInfo.m_QueryElements[m_Index].m_ComponentTypeID = ComponentStaticTypeID<T>::GetTypeID();
		m_Index++;
		m_QueryInfo.m_QueryElementsCount = m_Index;
		return *this;
	}

	inline QueryInfo QueryBuilder::Build() {
		return m_QueryInfo;
	}
}
