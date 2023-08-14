#pragma once

#include <Engine.h>
#include <CookieKat/Game/Game.h>

namespace CKE
{
	class GameEngine
	{
	public:
		GameEngine() = default;

		void Initialize();
		void Update();
		void Shutdown();

		inline Engine* GetEngine() { return &m_Engine; }

	private:
		Engine m_Engine{};
		Game m_Game{};
	};
}
