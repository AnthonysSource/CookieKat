#include "EntitySystem.h"

#include "CookieKat/Core/Profilling/Profilling.h"
#include "CookieKat/Core/Time/EngineTime.h"

#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Systems/ECS/Systems/ECSBaseSystem.h"
#include "CookieKat/Systems/Input/InputSystem.h"

#include "CookieKat/Engine/Entities/IWorldDefinition.h"
#include "CookieKat/Engine/Entities/Components/InputComponent.h"

namespace CKE {
	void EntitySystem::Initialize(SystemsRegistry& systemsRegistry) {
		m_pTaskSystem = systemsRegistry.GetSystem<TaskSystem>();

		m_EntityDatabase.Initialize(1'500'000);

		// Create the World
		auto pResourceSystem = systemsRegistry.GetSystem<ResourceSystem>();
		m_pWorldDefinition->LoadWorldResources(*pResourceSystem);
		m_pWorldDefinition->PopulateWorld(m_EntityDatabase, this);

		m_EntityDatabase.RegisterComponent<InputStateComponent>();
		m_EntityDatabase.AddSingletonComponent<InputStateComponent>({});
	}

	void EntitySystem::Update(EngineSystemUpdateContext& context) {
		CKE_PROFILE_EVENT();

		// Copy Input System Data into ECS Singleton Component
		{
			InputSystem* inputSys = context.GetEngineSystem<InputSystem>();
			InputContext inputContext{&inputSys->GetMouse(), &inputSys->GetKeyboard()};
			m_EntityDatabase.GetSingletonComponent<InputStateComponent>()->m_pInputContext = inputContext;
		}

		// Update all of the registered systems
		SystemUpdateContext sysUpdateContext{&m_EntityDatabase, m_pTaskSystem, context.GetEngineTime()->GetSecondsDeltaTime()};
		for (auto& pSystem : m_Systems) {
			pSystem->Update(sysUpdateContext);
		}
	}

	void EntitySystem::Shutdown() {
		for (ECSBaseSystem* pSystem : m_Systems) {
			CKE::Delete(pSystem);
		}
		m_Systems.clear();
	}

	void EntitySystem::SetWorldDefinition(IWorldDefinition* definition) {
		m_pWorldDefinition = definition;
	}
}
