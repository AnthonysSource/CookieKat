#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
// Utilities and Configuration
//-----------------------------------------------------------------------------

using namespace CKE;

struct DataComp1
{
	u32 a;
	u16 b;
	u16 c;
	u64 d;

	static DataComp1 DefaultValues() { return DataComp1{0, 1, 2, 3}; }

	friend bool operator==(const DataComp1& lhs, const DataComp1& rhs) {
		return lhs.a == rhs.a
				&& lhs.b == rhs.b
				&& lhs.c == rhs.c
				&& lhs.d == rhs.d;
	}

	friend bool operator!=(const DataComp1& lhs, const DataComp1& rhs) { return !(lhs == rhs); }
};

struct DataComp2
{
	i8  a;
	i32 b;
	i64 c;
	i16 d;

	static DataComp2 DefaultValues() { return DataComp2{0, 1, 2, 3}; }

	friend bool operator==(const DataComp2& lhs, const DataComp2& rhs) {
		return lhs.a == rhs.a
				&& lhs.b == rhs.b
				&& lhs.c == rhs.c
				&& lhs.d == rhs.d;
	}

	friend bool operator!=(const DataComp2& lhs, const DataComp2& rhs) { return !(lhs == rhs); }
};

struct Comp2
{
	i32 a = -53;
};

struct Comp3
{
	f64 a = 3.141516;
};

struct Comp4
{
	u8 a = 255;
};

// Base Test Config.
//-----------------------------------------------------------------------------

class EntityDatabase_T : public testing::Test
{
protected:
	void SetUp() override {
		m_EntityDB = EntityDatabase{};
		m_EntityDB.Initialize(MAX_ENTITIES);
		m_Debugger = m_EntityDB.GetDebugger();
		m_EntityDB.RegisterComponent<DataComp1>();
		m_EntityDB.RegisterComponent<DataComp2>();

		m_EntityDB.RegisterComponent<Comp2>();
		m_EntityDB.RegisterComponent<Comp3>();
		m_EntityDB.RegisterComponent<Comp4>();
	}

	void TearDown() override {
		m_EntityDB.Shutdown();
	}

	static constexpr u32   MAX_ENTITIES = 100;
	EntityDatabase         m_EntityDB{};
	EntityDatabaseDebugger m_Debugger{};
};

using Entities_T = EntityDatabase_T;
using Iterators_T = EntityDatabase_T;

//-----------------------------------------------------------------------------
// Tests
//-----------------------------------------------------------------------------

TEST_F(Entities_T, AddRemEntity_Simple) {
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);
	EntityID e1 = m_EntityDB.CreateEntity();
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 1);
	EntityID e2 = m_EntityDB.CreateEntity();
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 2);
	m_EntityDB.DeleteEntity(e1);
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 1);
	m_EntityDB.DeleteEntity(e2);
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);
}

TEST_F(Entities_T, AddRemove_MaxEntities) {
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);

	EntityID ids[MAX_ENTITIES];
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		ids[i] = m_EntityDB.CreateEntity();
	}

	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, MAX_ENTITIES);

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		m_EntityDB.DeleteEntity(ids[i]);
	}

	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);
}

TEST_F(Entities_T, AddingTooManyEntitiesOverflows) {
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		m_EntityDB.CreateEntity();
	}
	EXPECT_DEATH(m_EntityDB.CreateEntity(), "Assertion failed");
}

//-----------------------------------------------------------------------------

TEST_F(EntityDatabase_T, AddUnregisteredComponentFails) {
	EntityID e = m_EntityDB.CreateEntity();
	struct UnregisteredComp1
	{
		i32 a;
	};
	EXPECT_DEATH(m_EntityDB.AddComponent<UnregisteredComp1>(e, UnregisteredComp1{0}), "Assertion failed");
}

TEST_F(EntityDatabase_T, General) {
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);

	EntityID e1[13];
	EntityID e2[15];
	EntityID e3[7];

	for (EntityID& id : e1) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<Comp2>(id);
	}

	for (EntityID& id : e2) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<Comp3>(id);
		m_EntityDB.AddComponent<Comp2>(id);
	}

	for (EntityID& id : e3) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<Comp4>(id);
		m_EntityDB.AddComponent<Comp2>(id);
		m_EntityDB.AddComponent<Comp3>(id);
	}

	for (EntityID& id : e1) {
		EXPECT_EQ(m_EntityDB.GetComponent<Comp2>(id)->a, -53);
	}

	for (EntityID& id : e2) {
		EXPECT_EQ(m_EntityDB.GetComponent<Comp2>(id)->a, -53);
		EXPECT_EQ(m_EntityDB.GetComponent<Comp3>(id)->a, 3.141516);
	}

	for (EntityID& id : e3) {
		EXPECT_EQ(m_EntityDB.GetComponent<Comp2>(id)->a, -53);
		EXPECT_EQ(m_EntityDB.GetComponent<Comp3>(id)->a, 3.141516);
		EXPECT_EQ(m_EntityDB.GetComponent<Comp4>(id)->a, 255);
	}

	EntityDatabaseStateSnapshot snapshot = m_Debugger.GetStateSnapshot();

	EXPECT_EQ(snapshot.m_NumEntities, 35);
	EXPECT_GE(snapshot.m_NumArchetypes, 4);

	m_EntityDB.Shutdown();
}

TEST_F(EntityDatabase_T, EntityRecordGetsUpdatedWhenRemovingEntity) {
	EntityID  e1 = m_EntityDB.CreateEntity();
	DataComp1 c1{0, 1, 2, 3};
	m_EntityDB.AddComponent<DataComp1>(e1, c1);
	DataComp1* comp1 = m_EntityDB.GetComponent<DataComp1>(e1);
	EXPECT_TRUE(*comp1 == c1);

	EntityID  e2 = m_EntityDB.CreateEntity();
	DataComp1 c2{4, 5, 6, 7};
	m_EntityDB.AddComponent<DataComp1>(e2, c2);
	DataComp1* comp2 = m_EntityDB.GetComponent<DataComp1>(e2);
	EXPECT_TRUE(*comp2 == c2);

	m_EntityDB.DeleteEntity(e1);

	comp2 = m_EntityDB.GetComponent<DataComp1>(e2);
	EXPECT_TRUE(*comp2 == c2);
}

TEST_F(EntityDatabase_T, AddInterleavedEntities) {
	EntityID  e1 = m_EntityDB.CreateEntity();
	EntityID  e2 = m_EntityDB.CreateEntity();
	DataComp1 c1 = DataComp1{4, 5, 6, 7};
	DataComp1 c2 = DataComp1{0, 1, 2, 3};
	m_EntityDB.AddComponent<DataComp1>(e1, c1);
	m_EntityDB.AddComponent<DataComp1>(e2, c2);

	DataComp1* comp1 = m_EntityDB.GetComponent<DataComp1>(e1);
	EXPECT_TRUE(*comp1 == c1);
	DataComp1* comp2 = m_EntityDB.GetComponent<DataComp1>(e2);
	EXPECT_TRUE(*comp2 == c2);

	m_EntityDB.AddComponent<DataComp2>(e1);
	m_EntityDB.AddComponent<DataComp2>(e2);

	comp1 = m_EntityDB.GetComponent<DataComp1>(e1);
	EXPECT_TRUE(*comp1 == c1);
	comp2 = m_EntityDB.GetComponent<DataComp1>(e2);
	EXPECT_TRUE(*comp2 == c2);
}

//-----------------------------------------------------------------------------

TEST_F(Iterators_T, IterateOneComponent) {
	EntityID e1 = m_EntityDB.CreateEntity();
	EntityID e2 = m_EntityDB.CreateEntity();
	m_EntityDB.AddComponent<DataComp1>(e1, DataComp1{4, 5, 6, 7});
	m_EntityDB.AddComponent<DataComp2>(e1);
	m_EntityDB.AddComponent<DataComp1>(e2, DataComp1{0, 1, 2, 3});

	DataComp1 c{1, 1, 1, 1};
	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		ASSERT_NE(*pComp, c);
		*pComp = c;
	}

	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		EXPECT_TRUE(*pComp == c);
	}
}

TEST_F(Iterators_T, SingleComponentIterator) {
	Array<DataComp1, 50> data{};
	for (i32 i = 0; i < data.size(); ++i) {
		data[i] = DataComp1::DefaultValues();
		data[i].a += i;
		data[i].b += i;
		data[i].c += i;
		data[i].d += i;
	}

	for (i32 i = 0; i < 50; ++i) {
		EntityID e = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<DataComp1>(e, data[i]);
	}

	DataComp1 override_1{1, 1, 1, 1};

	i32 index = 0;
	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		ASSERT_EQ(*pComp, data[index]);
		*pComp = override_1;
		index++;
	}

	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		EXPECT_TRUE(*pComp == override_1);
	}
}

// Generate Single, Multi
// Access to values

TEST_F(Iterators_T, MultiComponentIterator) {
	// Prepare Data

	Array<DataComp1, 50> data1{};
	Array<DataComp2, 50> data2{};
	for (i32 i = 0; i < 50; ++i) {
		data1[i] = DataComp1::DefaultValues();
		data1[i].a += i;
		data1[i].b += i;
		data1[i].c += i;
		data1[i].d += i;

		data2[i] = DataComp2::DefaultValues();
		data2[i].a += i;
		data2[i].b += i;
		data2[i].c += i;
		data2[i].d += i;
	}

	for (i32 i = 0; i < 50; ++i) {
		EntityID e = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<DataComp1>(e, data1[i]);
		m_EntityDB.AddComponent<DataComp2>(e, data2[i]);
	}

	// Run Test

	DataComp1 override1{1, 1, 1, 1};
	DataComp2 override2{2, 2, 2, 2};

	i32 index = 0;
	for (ComponentTuple* pTuple : m_EntityDB.GetMultiCompIter<DataComp1, DataComp2>()) {
		DataComp1* pComp1 = static_cast<DataComp1*>(pTuple->GetComponent(0));
		DataComp2* pComp2 = static_cast<DataComp2*>(pTuple->GetComponent(1));

		EXPECT_EQ(*pComp1, data1[index]);
		EXPECT_EQ(*pComp2, data2[index]);

		*pComp1 = override1;
		*pComp2 = override2;

		index++;
	}

	for (ComponentTuple* pTuple : m_EntityDB.GetMultiCompIter<DataComp1, DataComp2>()) {
		DataComp1* pComp1 = static_cast<DataComp1*>(pTuple->GetComponent(0));
		DataComp2* pComp2 = static_cast<DataComp2*>(pTuple->GetComponent(1));

		EXPECT_TRUE(*pComp1 == override1);
		EXPECT_TRUE(*pComp2 == override2);
	}
}

TEST_F(Iterators_T, IterateMultiSingleComponent) {
	EntityID e1 = m_EntityDB.CreateEntity();
	EntityID e2 = m_EntityDB.CreateEntity();
	m_EntityDB.AddComponent<DataComp1>(e1, DataComp1{4, 5, 6, 7});
	m_EntityDB.AddComponent<DataComp1>(e2, DataComp1{0, 1, 2, 3});

	DataComp1 c{1, 1, 1, 1};
	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		ASSERT_NE(*pComp, c);
		*pComp = c;
	}

	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		EXPECT_TRUE(*pComp == c);
	}
}

TEST_F(Iterators_T, EntityComponentIterator) {
	Array<EntityID, 2> e;
	e[0] = m_EntityDB.CreateEntity();
	e[1] = m_EntityDB.CreateEntity();
	Array<DataComp1, 2> d;
	d[0] = DataComp1{4, 5, 6, 7};
	d[1] = DataComp1{0, 1, 2, 3};
	m_EntityDB.AddComponent<DataComp1>(e[0], d[0]);
	m_EntityDB.AddComponent<DataComp1>(e[1], d[1]);

	// This works because we know that entities get stored in order in this case
	i32 idx = 0;
	for (EntityComponentPair* pData : m_EntityDB.GetEntityIterator(ComponentStaticTypeID<DataComp1>::s_CompID)) {
		ASSERT_TRUE(pData->m_EntityID == e[idx]);
		ASSERT_TRUE(*reinterpret_cast<DataComp1*>(pData->m_pComponent) == d[idx]);
		idx++;
	}
}

TEST_F(Iterators_T, EmptySingleIterator) {
	for (DataComp1* pComp : m_EntityDB.GetSingleCompIter<DataComp1>()) {
		ASSERT_TRUE(false);
		pComp->a += 1;
	}
	ASSERT_TRUE(true);
}

TEST_F(Iterators_T, EmptyMultiIterator) {
	for (auto pComp : m_EntityDB.GetMultiCompIter<DataComp1, DataComp2>()) {
		ASSERT_TRUE(false);
	}
	ASSERT_TRUE(true);
}


//-----------------------------------------------------------------------------
