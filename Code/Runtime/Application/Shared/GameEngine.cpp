#include "GameEngine.h"

namespace CKE
{
	void GameEngine::Initialize()
	{
		m_Engine.InitializeCore();
		m_Engine.GetEntitySystem()->SetWorldDefinition(&m_Game);
		m_Engine.InitializeEngine();
	}

	void GameEngine::Update()
	{
		m_Engine.Update();
	}

	void GameEngine::Shutdown()
	{
		m_Engine.Shutdown();
	}
}
