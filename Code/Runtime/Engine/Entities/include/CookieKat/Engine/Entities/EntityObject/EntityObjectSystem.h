#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"

namespace CKE::EntityObject {

	// Entity Object System
	// High-Level object-oriented entity system API
	// This is just a set of auxiliary classes that wrap the ECS model into
	// an object model closer to the one in Unity

	class Entity
	{
	public:
		Entity(EntityDatabase* pEntityDB);

		template <typename T>
		void AddComponent(EntityID entity, T& component);

		template <typename T>
		void RemoveComponent(EntityID entity);

		template <typename T>
		T* GetComponent(EntityID entity);

		template <typename T>
		bool HasComponent(EntityID entity);

	private:
		EntityDatabase* m_EntityDB = nullptr;
		EntityID        m_ID{};
	};
}

namespace CKE::EntityObject {
	inline Entity::Entity(EntityDatabase* pEntityDB) {
		m_ID = pEntityDB->CreateEntity();
		m_EntityDB = pEntityDB;
	}

	template <typename T>
	void Entity::AddComponent(EntityID entity, T& component) {
		m_EntityDB->AddComponent<T>(m_EntityDB, component);
	}

	template <typename T>
	void Entity::RemoveComponent(EntityID entity) {
		m_EntityDB->RemoveComponent<T>(entity);
	}

	template <typename T>
	T* Entity::GetComponent(EntityID entity) {
		return m_EntityDB->GetComponent<T>(entity);
	}

	template <typename T>
	bool Entity::HasComponent(EntityID entity) {
		return m_EntityDB->HasComponent<T>(m_ID);
	}
}
