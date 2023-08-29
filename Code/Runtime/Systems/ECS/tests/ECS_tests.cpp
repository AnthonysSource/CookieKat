#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
// Utilities and Configuration
//-----------------------------------------------------------------------------

using namespace CKE;

struct ComplexT1_Component
{
	u32 a;
	u16 b;
	u16 c;
	u64 d;

	static ComplexT1_Component DefaultTestValues() { return ComplexT1_Component{0, 1, 2, 3}; }

	friend bool operator==(const ComplexT1_Component& lhs, const ComplexT1_Component& rhs) {
		return lhs.a == rhs.a
				&& lhs.b == rhs.b
				&& lhs.c == rhs.c
				&& lhs.d == rhs.d;
	}

	friend bool operator!=(const ComplexT1_Component& lhs, const ComplexT1_Component& rhs) { return !(lhs == rhs); }
};

struct ComplexT2_Component
{
	i8  a;
	i32 b;
	i64 c;
	i16 d;

	static ComplexT2_Component DefaultTestValues() { return ComplexT2_Component{0, 1, 2, 3}; }

	friend bool operator==(const ComplexT2_Component& lhs, const ComplexT2_Component& rhs) {
		return lhs.a == rhs.a
				&& lhs.b == rhs.b
				&& lhs.c == rhs.c
				&& lhs.d == rhs.d;
	}

	friend bool operator!=(const ComplexT2_Component& lhs, const ComplexT2_Component& rhs) { return !(lhs == rhs); }
};

struct I32_Component
{
	i32 a = -53;
};

struct F64_Component
{
	f64 a = 3.141516;
};

struct U8_Component
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
		m_EntityDB.RegisterComponent<ComplexT1_Component>();
		m_EntityDB.RegisterComponent<ComplexT2_Component>();

		m_EntityDB.RegisterComponent<I32_Component>();
		m_EntityDB.RegisterComponent<F64_Component>();
		m_EntityDB.RegisterComponent<U8_Component>();
	}

	void TearDown() override {
		m_EntityDB.Shutdown();
	}

	static constexpr u32   MAX_ENTITIES = 1000;
	EntityDatabase         m_EntityDB{};
	EntityDatabaseDebugger m_Debugger{};
};

using Entities_T = EntityDatabase_T;
using Iterators_T = EntityDatabase_T;
using Queries_T = EntityDatabase_T;

// Utilities
//-----------------------------------------------------------------------------

struct ConfigurationInfo
{
	Vector<EntityID> m_EntitiesA;
	Vector<EntityID> m_EntitiesB;
	Vector<EntityID> m_EntitiesC;
	Vector<EntityID> m_EntitiesD;
};

void DefaultComponentConfiguration_2(EntityDatabase&   m_EntityDB,
                                     Vector<EntityID>& entitiesA,
                                     Vector<EntityID>& entitiesB,
                                     Vector<EntityID>& entitiesC,
                                     Vector<EntityID>& entitiesD) {
	for (EntityID& id : entitiesA) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<I32_Component>(id);
	}

	for (EntityID& id : entitiesB) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<F64_Component>(id);
		m_EntityDB.AddComponent<I32_Component>(id);
	}

	for (EntityID& id : entitiesC) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<U8_Component>(id);
		m_EntityDB.AddComponent<I32_Component>(id);
		m_EntityDB.AddComponent<F64_Component>(id);
	}

	for (EntityID& id : entitiesD) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<U8_Component>(id);
		m_EntityDB.AddComponent<F64_Component>(id);
	}
}

ConfigurationInfo DefaultComponentConfiguration(EntityDatabase& entityDB) {
	ConfigurationInfo cInfo{};
	cInfo.m_EntitiesA.resize(100);
	cInfo.m_EntitiesB.resize(100);
	cInfo.m_EntitiesC.resize(100);
	cInfo.m_EntitiesD.resize(100);
	DefaultComponentConfiguration_2(entityDB,
	                                cInfo.m_EntitiesA,
	                                cInfo.m_EntitiesB,
	                                cInfo.m_EntitiesC,
	                                cInfo.m_EntitiesD);
	return cInfo;
}

//-----------------------------------------------------------------------------
// Tests
//-----------------------------------------------------------------------------

TEST_F(Entities_T, Add_Remove_Entity_Simple) {
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

TEST_F(Entities_T, Add_Remove_To_Max_Entities_Limit) {
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

TEST_F(Entities_T, Add_Entity_Over_Max_Crashes) {
	EXPECT_EQ(m_Debugger.GetStateSnapshot().m_NumEntities, 0);
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		m_EntityDB.CreateEntity();
	}
	EXPECT_DEATH(m_EntityDB.CreateEntity(), "Assertion failed");
}

//-----------------------------------------------------------------------------

TEST_F(EntityDatabase_T, Add_Unregistered_Component_Fails) {
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
		m_EntityDB.AddComponent<I32_Component>(id);
	}

	for (EntityID& id : e2) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<F64_Component>(id);
		m_EntityDB.AddComponent<I32_Component>(id);
	}

	for (EntityID& id : e3) {
		id = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<U8_Component>(id);
		m_EntityDB.AddComponent<I32_Component>(id);
		m_EntityDB.AddComponent<F64_Component>(id);
	}

	for (EntityID& id : e1) {
		EXPECT_EQ(m_EntityDB.GetComponent<I32_Component>(id)->a, -53);
	}

	for (EntityID& id : e2) {
		EXPECT_EQ(m_EntityDB.GetComponent<I32_Component>(id)->a, -53);
		EXPECT_EQ(m_EntityDB.GetComponent<F64_Component>(id)->a, 3.141516);
	}

	for (EntityID& id : e3) {
		EXPECT_EQ(m_EntityDB.GetComponent<I32_Component>(id)->a, -53);
		EXPECT_EQ(m_EntityDB.GetComponent<F64_Component>(id)->a, 3.141516);
		EXPECT_EQ(m_EntityDB.GetComponent<U8_Component>(id)->a, 255);
	}

	EntityDatabaseStateSnapshot snapshot = m_Debugger.GetStateSnapshot();

	EXPECT_EQ(snapshot.m_NumEntities, 35);
	EXPECT_GE(snapshot.m_NumArchetypes, 4);

	m_EntityDB.Shutdown();
}

TEST_F(EntityDatabase_T, EntityRecord_Update_On_RemoveEntity) {
	EntityID            e1 = m_EntityDB.CreateEntity();
	ComplexT1_Component c1{0, 1, 2, 3};
	m_EntityDB.AddComponent<ComplexT1_Component>(e1, c1);
	ComplexT1_Component* comp1 = m_EntityDB.GetComponent<ComplexT1_Component>(e1);
	EXPECT_TRUE(*comp1 == c1);

	EntityID            e2 = m_EntityDB.CreateEntity();
	ComplexT1_Component c2{4, 5, 6, 7};
	m_EntityDB.AddComponent<ComplexT1_Component>(e2, c2);
	ComplexT1_Component* comp2 = m_EntityDB.GetComponent<ComplexT1_Component>(e2);
	EXPECT_TRUE(*comp2 == c2);

	m_EntityDB.DeleteEntity(e1);

	comp2 = m_EntityDB.GetComponent<ComplexT1_Component>(e2);
	EXPECT_TRUE(*comp2 == c2);
}

TEST_F(EntityDatabase_T, Add_Interleaved_Components_To_Entities) {
	EntityID            e1 = m_EntityDB.CreateEntity();
	EntityID            e2 = m_EntityDB.CreateEntity();
	ComplexT1_Component c1 = ComplexT1_Component{4, 5, 6, 7};
	ComplexT1_Component c2 = ComplexT1_Component{0, 1, 2, 3};
	m_EntityDB.AddComponent<ComplexT1_Component>(e1, c1);
	m_EntityDB.AddComponent<ComplexT1_Component>(e2, c2);

	ComplexT1_Component* comp1 = m_EntityDB.GetComponent<ComplexT1_Component>(e1);
	EXPECT_TRUE(*comp1 == c1);
	ComplexT1_Component* comp2 = m_EntityDB.GetComponent<ComplexT1_Component>(e2);
	EXPECT_TRUE(*comp2 == c2);

	m_EntityDB.AddComponent<ComplexT2_Component>(e1);
	m_EntityDB.AddComponent<ComplexT2_Component>(e2);

	comp1 = m_EntityDB.GetComponent<ComplexT1_Component>(e1);
	EXPECT_TRUE(*comp1 == c1);
	comp2 = m_EntityDB.GetComponent<ComplexT1_Component>(e2);
	EXPECT_TRUE(*comp2 == c2);
}

//-----------------------------------------------------------------------------
// Queries
//-----------------------------------------------------------------------------

TEST_F(Queries_T, Query_2_Components) {
	ConfigurationInfo c = DefaultComponentConfiguration(m_EntityDB);

	QueryResult result{};
	m_EntityDB.Query(QueryBuilder{}.Add<I32_Component>()
	                               .Add<F64_Component>()
	                               .Build(), &result);

	EXPECT_EQ(result.m_TotalEntities, 200);
	EXPECT_EQ(result.m_MatchingArchetypes[0].m_TotalRows, 100);
	EXPECT_EQ(result.m_MatchingArchetypes[0].m_ComponentColumns[0], 1);
	EXPECT_EQ(result.m_MatchingArchetypes[0].m_ComponentColumns[1], 2);

	EXPECT_EQ(result.m_MatchingArchetypes[1].m_TotalRows, 100);
	EXPECT_EQ(result.m_MatchingArchetypes[1].m_ComponentColumns[0], 1);
	EXPECT_EQ(result.m_MatchingArchetypes[1].m_ComponentColumns[1], 0);
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

TEST_F(Iterators_T, Iterator_Component_Single_2) {
	EntityID e1 = m_EntityDB.CreateEntity();
	EntityID e2 = m_EntityDB.CreateEntity();
	m_EntityDB.AddComponent<ComplexT1_Component>(e1, ComplexT1_Component{4, 5, 6, 7});
	m_EntityDB.AddComponent<ComplexT2_Component>(e1);
	m_EntityDB.AddComponent<ComplexT1_Component>(e2, ComplexT1_Component{0, 1, 2, 3});

	ComplexT1_Component c{1, 1, 1, 1};
	for (ComplexT1_Component* pComp : m_EntityDB.GetSingleCompIter<ComplexT1_Component>()) {
		ASSERT_NE(*pComp, c);
		*pComp = c;
	}

	for (ComplexT1_Component* pComp : m_EntityDB.GetSingleCompIter<ComplexT1_Component>()) {
		EXPECT_TRUE(*pComp == c);
	}
}

TEST_F(Iterators_T, Iterator_Component_Single) {
	Array<ComplexT1_Component, 50> data{};
	for (i32 i = 0; i < data.size(); ++i) {
		data[i] = ComplexT1_Component::DefaultTestValues();
		data[i].a += i;
		data[i].b += i;
		data[i].c += i;
		data[i].d += i;
	}

	for (i32 i = 0; i < 50; ++i) {
		EntityID e = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<ComplexT1_Component>(e, data[i]);
	}

	ComplexT1_Component override_1{1, 1, 1, 1};

	i32 index = 0;
	for (ComplexT1_Component* pComp : m_EntityDB.GetSingleCompIter<ComplexT1_Component>()) {
		ASSERT_EQ(*pComp, data[index]);
		*pComp = override_1;
		index++;
	}

	for (ComplexT1_Component* pComp : m_EntityDB.GetSingleCompIter<ComplexT1_Component>()) {
		EXPECT_TRUE(*pComp == override_1);
	}
}

TEST_F(Iterators_T, Iterator_Component_Multi_Template) {
	// Prepare Data

	Array<ComplexT1_Component, 50> data1{};
	Array<ComplexT2_Component, 50> data2{};
	for (i32 i = 0; i < 50; ++i) {
		data1[i] = ComplexT1_Component::DefaultTestValues();
		data1[i].a += i;
		data1[i].b += i;
		data1[i].c += i;
		data1[i].d += i;

		data2[i] = ComplexT2_Component::DefaultTestValues();
		data2[i].a += i;
		data2[i].b += i;
		data2[i].c += i;
		data2[i].d += i;
	}

	for (i32 i = 0; i < 50; ++i) {
		EntityID e = m_EntityDB.CreateEntity();
		m_EntityDB.AddComponent<ComplexT1_Component>(e, data1[i]);
		m_EntityDB.AddComponent<ComplexT2_Component>(e, data2[i]);
	}

	// Run Test

	ComplexT1_Component override1{1, 1, 1, 1};
	ComplexT2_Component override2{2, 2, 2, 2};

	i32 index = 0;
	for (ComponentTuple* pTuple : m_EntityDB.GetMultiCompIter<ComplexT1_Component, ComplexT2_Component>()) {
		ComplexT1_Component* pComp1 = static_cast<ComplexT1_Component*>(pTuple->GetComponent(0));
		ComplexT2_Component* pComp2 = static_cast<ComplexT2_Component*>(pTuple->GetComponent(1));

		EXPECT_EQ(*pComp1, data1[index]);
		EXPECT_EQ(*pComp2, data2[index]);

		*pComp1 = override1;
		*pComp2 = override2;

		index++;
	}

	for (ComponentTuple* pTuple : m_EntityDB.GetMultiCompIter<ComplexT1_Component, ComplexT2_Component>()) {
		ComplexT1_Component* pComp1 = static_cast<ComplexT1_Component*>(pTuple->GetComponent(0));
		ComplexT2_Component* pComp2 = static_cast<ComplexT2_Component*>(pTuple->GetComponent(1));

		EXPECT_TRUE(*pComp1 == override1);
		EXPECT_TRUE(*pComp2 == override2);
	}
}

TEST_F(Iterators_T, Iterator_Component_Multi_For_Single) {
	EntityID e1 = m_EntityDB.CreateEntity();
	EntityID e2 = m_EntityDB.CreateEntity();
	m_EntityDB.AddComponent<ComplexT1_Component>(e1, ComplexT1_Component{4, 5, 6, 7});
	m_EntityDB.AddComponent<ComplexT1_Component>(e2, ComplexT1_Component{0, 1, 2, 3});

	ComplexT1_Component c{1, 1, 1, 1};
	for (ComponentTuple* pCompTuple : m_EntityDB.GetMultiCompIter<ComplexT1_Component>()) {
		ComplexT1_Component* pComp = pCompTuple->GetComponent<ComplexT1_Component>(0);
		ASSERT_NE(*pComp, c);
		*pComp = c;
	}

	for (ComponentTuple* pCompTuple : m_EntityDB.GetMultiCompIter<ComplexT1_Component>()) {
		ComplexT1_Component* pComp = pCompTuple->GetComponent<ComplexT1_Component>(0);
		EXPECT_TRUE(*pComp == c);
	}
}

TEST_F(Iterators_T, Iterator_Component_Entity) {
	Array<EntityID, 2> e;
	e[0] = m_EntityDB.CreateEntity();
	e[1] = m_EntityDB.CreateEntity();
	Array<ComplexT1_Component, 2> d;
	d[0] = ComplexT1_Component{4, 5, 6, 7};
	d[1] = ComplexT1_Component{0, 1, 2, 3};
	m_EntityDB.AddComponent<ComplexT1_Component>(e[0], d[0]);
	m_EntityDB.AddComponent<ComplexT1_Component>(e[1], d[1]);

	// This works because we know that entities get stored in order in this case
	i32 idx = 0;
	for (EntityComponentPair* pData : m_EntityDB.GetEntityIterator(ComponentStaticTypeID<ComplexT1_Component>::s_CompID)) {
		ASSERT_TRUE(pData->m_EntityID == e[idx]);
		ASSERT_TRUE(*reinterpret_cast<ComplexT1_Component*>(pData->m_pComponent) == d[idx]);
		idx++;
	}
}

TEST_F(Iterators_T, Iterator_Single_Empty) {
	for (ComplexT1_Component* pComp : m_EntityDB.GetSingleCompIter<ComplexT1_Component>()) {
		ASSERT_TRUE(false);
	}
	ASSERT_TRUE(true);
}

TEST_F(Iterators_T, Iterator_Single_Multi) {
	for (auto pComp : m_EntityDB.GetMultiCompIter<ComplexT1_Component, ComplexT2_Component>()) {
		ASSERT_TRUE(false);
	}
	ASSERT_TRUE(true);
}

//-----------------------------------------------------------------------------
