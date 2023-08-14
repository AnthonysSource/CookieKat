#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"

namespace CKE
{
	class ResourceSystem;
	class EntitySystem;

	class IWorldDefinition
	{
	public:
		virtual void LoadWorldResources(ResourceSystem& resources) = 0;
		virtual void PopulateWorld(EntityDatabase& e, EntitySystem* system) = 0;
	};
}
