#pragma once

#include "CookieKat/Systems/EngineSystem/SystemsRegistry.h"
#include "CookieKat/Core/Platform/Concepts.h"

//-----------------------------------------------------------------------------

namespace CKE
{
	class EngineTime;
	class IEngineSystem;
}

//-----------------------------------------------------------------------------

namespace CKE
{
	class EngineSystemUpdateContext
	{
		friend class Engine;

	public:
		template <typename T>
			requires IsDerived<IEngineSystem, T>
		inline T* GetEngineSystem() const { return m_pSystemsRegistry->GetSystem<T>(); }

		inline EngineTime* GetEngineTime() const { return m_pEngineTime; }

	private:
		SystemsRegistry* m_pSystemsRegistry = nullptr;

		EngineTime* m_pEngineTime = nullptr;
	};
}
