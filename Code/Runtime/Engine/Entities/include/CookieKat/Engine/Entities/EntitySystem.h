#pragma once

#include "API.h"
#include "CookieKat/Systems/EngineSystem/EngineSystemUpdateContext.h"
#include "CookieKat/Systems/EngineSystem/SystemsRegistry.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/ECS/Systems/ECSBaseSystem.h"
#include "CookieKat/Systems/TaskSystem/TaskSystem.h"

#include "CookieKat/Core/Memory/Memory.h"

namespace CKE
{
	class EntityDatabase;
	class IWorldDefinition;
}

namespace CKE
{
	class CKE_API EntitySystem : public IEngineSystem
	{
	public:
		// Lifetime
		//-----------------------------------------------------------------------------

		void Initialize(SystemsRegistry& systemsRegistry);
		void Update(EngineSystemUpdateContext& context);
		void Shutdown();

		//-----------------------------------------------------------------------------

		inline EntityDatabase* GetEntityDatabase() { return &m_EntityDatabase; }

		// Sets the world definition that will be used to create the world at startup
		void SetWorldDefinition(IWorldDefinition* definition);

		//-----------------------------------------------------------------------------

		template <typename T>
		void AddSystem()
		{
			T* pSys = CKE::New<T>();
			m_Systems.emplace_back(pSys);
		}

	private:
		EntityDatabase m_EntityDatabase;

		Vector<ECSBaseSystem*> m_Systems;

		TaskSystem* m_pTaskSystem = nullptr;
		IWorldDefinition* m_pWorldDefinition = nullptr;
	};
}
