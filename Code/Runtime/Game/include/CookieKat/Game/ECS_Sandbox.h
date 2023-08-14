#pragma once

#include "CookieKat/Systems/ECS/EntityDatabase.h"

namespace CKE {
	namespace Game {
		struct Position {};
		struct EulerRotation{};
		struct Scale{};
		struct LocalToWorld{};

		struct Collectable{};
		struct EconomicValue{};
		struct BoxCollider{};

		struct PlayerTag{};
		struct CharacterMovement2D{};

		struct RenderSprite{};

		struct FlagOwner
		{
			EntityID m_Owner;
		};

		void Coin() {
			Position pos;
			EulerRotation er;
			LocalToWorld l2w;

			Collectable c;
			EconomicValue ev;
			BoxCollider bc;

			RenderSprite rs;
		}

		void Player() {
			Position pos;
			EulerRotation er;
			LocalToWorld l2w;

			BoxCollider bc;
			PlayerTag pt;
			CharacterMovement2D cm2d;

			RenderSprite rs;
		}

		void Flag() {
			Position pos;
			EulerRotation er;
			LocalToWorld l2w;

			BoxCollider bc;
			FlagOwner ow;
		}

		// Query All Flags assigned to an entity
		//   - Foreach(Flag) Owner == Entity
		//   - Map<Flag, Vector<Entity>> && Map<Entity, Vector<Flag>>
		//
		// Insights:
		//   - Relationships don't trigger structural changes
		//   - Every Relationship can be efficiently modeled as a Map
		//   - Relationships can be added as a separate system
		//
		// EntityDatabase
		//   - Handles Entity data layout
		//   - Organizes it in memory in different ways
	}
}
