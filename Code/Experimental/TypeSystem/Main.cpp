#include "CookieKat/Core/Platform/PlatformTime.h"

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Platform/Asserts.h"

#include <iostream>

using namespace CKE;

//-----------------------------------------------------------------------------

class TypeInfo
{
public:
	inline u64                GetTypeID() const { return m_TypeID; }
	inline char const*        GetTypeName() const { return m_pTypeName; }
	inline Vector<u64> const& GetParents() const { return m_Parents; }

protected:
	char const* m_pTypeName = nullptr;
	u64         m_TypeID = 0;
	Vector<u64> m_Parents;
};

template <typename T>
class TTypeInfo : public TypeInfo {};

//-----------------------------------------------------------------------------

class IReflectedType
{
public:
	inline static TypeInfo const* s_pTypeInfo;

	inline virtual TypeInfo const* GetTypeInfo() { return s_pTypeInfo; }
	inline virtual u64             GetTypeID() { return s_pTypeInfo->GetTypeID(); }
};

#define REFLECT(x) \
public: \
	inline static TypeInfo const* s_pTypeInfo;\
	\
	inline static TypeInfo const* GetStaticTypeInfo() {return s_pTypeInfo; } \
	inline static u64 GetStaticTypeID() {return s_pTypeInfo->GetTypeID();} \
	\
	inline virtual TypeInfo const* GetTypeInfo() override {return s_pTypeInfo;} \
	inline virtual u64 GetTypeID() override {return s_pTypeInfo->GetTypeID();}

class GameObject : IReflectedType
{
	REFLECT(GameObject)
};

class Actor : public GameObject
{
	REFLECT(Actor)

public:
	i32 m_ActorData{42};
};

class RandomClass : public IReflectedType
{
	REFLECT(RandomClass)
};

//-----------------------------------------------------------------------------

template <>
class TTypeInfo<GameObject> : public TypeInfo
{
public:
	static void Register() {
		auto pTypeInfo = new TTypeInfo<GameObject>();
		pTypeInfo->m_pTypeName = "CKE::GameObject";
		pTypeInfo->m_TypeID = std::hash<String>()(pTypeInfo->m_pTypeName);
		GameObject::s_pTypeInfo = pTypeInfo;
	}
};

template <>
class TTypeInfo<Actor> : public TypeInfo
{
public:
	static void Register() {
		auto pTypeInfo = new TTypeInfo<Actor>();
		pTypeInfo->m_pTypeName = "CKE::Actor";
		pTypeInfo->m_TypeID = std::hash<String>()(pTypeInfo->m_pTypeName);
		pTypeInfo->m_Parents.emplace_back(GameObject::GetStaticTypeID());
		Actor::s_pTypeInfo = pTypeInfo;
	}
};

template <>
class TTypeInfo<RandomClass> : public TypeInfo
{
public:
	static void Register() {
		auto pTypeInfo = new TTypeInfo<RandomClass>();
		pTypeInfo->m_pTypeName = "CKE::RandomClass";
		pTypeInfo->m_TypeID = std::hash<String>()(pTypeInfo->m_pTypeName);
		RandomClass::s_pTypeInfo = pTypeInfo;
	}
};

//-----------------------------------------------------------------------------

template <typename T, typename K>
T* Cast(K* obj) {
	if (obj->GetTypeID() == T::GetStaticTypeID()) { return reinterpret_cast<T*>(obj); }

	auto vec = obj->GetTypeInfo()->GetParents();
	for (auto parentID : vec) {
		if (parentID == T::GetStaticTypeID()) {
			return reinterpret_cast<T*>(obj);
		}
	}
	CKE_UNREACHABLE_CODE();
}

template <typename T, typename K>
T* TryCast(K* obj) {
	if (obj->GetTypeID() == T::GetStaticTypeID()) { return reinterpret_cast<T*>(obj); }

	auto vec = obj->GetTypeInfo()->GetParents();
	for (auto parentID : vec) {
		if (parentID == T::GetStaticTypeID()) {
			return reinterpret_cast<T*>(obj);
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------

int main() {
	TTypeInfo<GameObject>::Register();
	TTypeInfo<Actor>::Register();
	TTypeInfo<RandomClass>::Register();

	Actor       actor;
	GameObject  go;
	RandomClass ra;

	std::cout << actor.GetTypeInfo()->GetTypeName() << " / " << actor.GetTypeID() << "\n";
	std::cout << go.GetTypeInfo()->GetTypeName() << " / " << go.GetTypeID() << "\n";

	GameObject* pGo = &actor;
	Actor*      pActor = Cast<Actor>(pGo);
	// RandomClass* pRa = Cast<RandomClass>(pGo); // Should assert

	return 0;
}
