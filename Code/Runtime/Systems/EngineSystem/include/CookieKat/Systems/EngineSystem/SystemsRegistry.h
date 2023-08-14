#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/Concepts.h"
#include "CookieKat/Core/Containers/Containers.h"

#include "CookieKat/Systems/EngineSystem/IEngineSystem.h"

namespace CKE
{
	// Registry that contains all of the engine systems
	class SystemsRegistry
	{
		friend class Engine;

	public:
		template <typename T>
			requires IsDerived<IEngineSystem, T>
		T* GetSystem()
		{
			EngineSystemID id = typeid(T).hash_code();
			//CKE_ASSERT(!m_EngineSystems.contains(id));
			auto sysPair = m_EngineSystems.find(id);
			if (sysPair != m_EngineSystems.end())
			{
				return static_cast<T*>(sysPair->second);
			}
			CKE_UNREACHABLE_CODE();
			return nullptr;
		}

	private:
		template <typename T>
			requires IsDerived<IEngineSystem, T>
		void RegisterSystem(T* system)
		{
			EngineSystemID id = typeid(T).hash_code();
			CKE_ASSERT(!m_EngineSystems.contains(id)); // Avoid double registration
			m_EngineSystems.insert({id, system});
		}

	private:
		Map<EngineSystemID, IEngineSystem*> m_EngineSystems;
	};
}
