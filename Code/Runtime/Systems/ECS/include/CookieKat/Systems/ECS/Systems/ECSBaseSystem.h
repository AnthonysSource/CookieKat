#pragma once

#include "../IDs.h"

namespace CKE {
	// Forward declarations
	class TaskSystem;
	class EntityDatabase;
}

namespace CKE {
	// Available data inside a system update method
	class SystemUpdateContext
	{
	public:
		SystemUpdateContext(EntityDatabase* pAdmin, TaskSystem* pTaskSystem, f32 dt) :
			m_pAdmin(pAdmin), m_pTaskSystem(pTaskSystem), m_DeltaTime(dt) { }

		inline EntityDatabase* GetEntityDatabase() { return m_pAdmin; }
		inline TaskSystem*     GetTaskSystem() { return m_pTaskSystem; }

		// Returns the time it took to process the previous frame
		inline f32 GetDeltaTime() const { return m_DeltaTime; }

	private:
		EntityDatabase* m_pAdmin;
		TaskSystem*     m_pTaskSystem;
		f32             m_DeltaTime;
	};

	//-----------------------------------------------------------------------------

	// Interface for all ECS systems
	class ECSBaseSystem
	{
	public:
		virtual ~ECSBaseSystem() = default;

		virtual void Initialize() { }
		virtual void Update(SystemUpdateContext ctx) { }
		virtual void Shutdown() { }
	};

	//-----------------------------------------------------------------------------

	// TODO: In dev.
	// Component access information
	struct SystemAccessData
	{
		Vector<ComponentTypeID> m_ReadOnlyComps;
		Vector<ComponentTypeID> m_ReadWriteComps;
	};
}
