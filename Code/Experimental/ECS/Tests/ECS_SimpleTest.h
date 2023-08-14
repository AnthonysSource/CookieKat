#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/Asserts.h"

#include "CookieKat/Systems/ECS/Archetype.h"
#include "CookieKat/Systems/ECS/IDs.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "BaseComponents.h"
#include "Base_Test.h"

#include <iostream>
#include <chrono>
#include <functional>

namespace CKE::ECS
{
	using namespace CKE;

	class ECS_SimpleTest
	{
	public:
		ECS_SimpleTest() = default;

		void CreateDestroyComponentsTest()
		{
			EntityID entityID = m_Admin.CreateEntity();
			m_Admin.AddComponent<VelocityComponent>(entityID, 1.0f, 1.0f, 1.0f);
			m_Admin.AddComponent<AccelerationComponent>(entityID, 2.0f, 2.0f, 2.0f);

			m_Admin.PrintEntityState(entityID);

			m_Admin.RemoveComponent<AccelerationComponent>(entityID);

			m_Admin.PrintEntityState(entityID);

			m_Admin.AddComponent<AccelerationComponent>(entityID, 3.0f, 3.0f, 3.0f);

			m_Admin.PrintEntityState(entityID);

			m_Admin.PrintAdminState();
			m_Admin.DeleteEntity(entityID);
			m_Admin.PrintAdminState();
		}

		void Run()
		{
			m_Admin.RegisterComponent<PositionComponent>();
			m_Admin.RegisterComponent<AccelerationComponent>();
			m_Admin.RegisterComponent<VelocityComponent>();

			CreateDestroyComponentsTest();

			for (int i = 0; i < 1; ++i)
			{
				EntityID entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entityID);
			}

			for (int i = 0; i < 5; ++i)
			{
				EntityID entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<AccelerationComponent>(entityID);

				entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<AccelerationComponent>(entityID);

				entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<VelocityComponent>(entityID);
			}

			for (int i = 0; i < 3; ++i)
			{
				EntityID entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<AccelerationComponent>(entityID);
				m_Admin.AddComponent<PositionComponent>(entityID);

				entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entityID);
				m_Admin.AddComponent<AccelerationComponent>(entityID);
			}

			for (int i = 0; i < 2; ++i)
			{
				EntityID entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<AccelerationComponent>(entityID);
				m_Admin.AddComponent<VelocityComponent>(entityID);
				m_Admin.AddComponent<PositionComponent>(entityID);

				entityID = m_Admin.CreateEntity();
				m_Admin.AddComponent<PositionComponent>(entityID);
				m_Admin.AddComponent<AccelerationComponent>(entityID);
				m_Admin.AddComponent<VelocityComponent>(entityID);
			}

			m_Admin.PrintAdminState();
		}

	private:
		EntityDatabase m_Admin{100};
	};
}