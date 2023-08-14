#pragma once

#include "CookieKat/Systems/EngineSystem/SystemsRegistry.h"

#include "CookieKat/Core/Time/EngineTime.h"
#include "CookieKat/Systems/Input/InputSystem.h"
#include "CookieKat/Systems/TaskSystem/TaskSystem.h"

#include "CookieKat/Systems/Resources/ResourceSystem.h"
#include "CookieKat/Engine/Resources/Loaders/TextureLoader.h"
#include "CookieKat/Engine/Resources/Loaders/PipelineLoader.h"
#include "CookieKat/Engine/Resources/Loaders/MeshLoader.h"
#include "CookieKat/Engine/Resources/Loaders/MaterialLoader.h"

#include "CookieKat/Engine/Render/RenderingSystem.h"

#include "CookieKat/Engine/Entities/EntitySystem.h"

namespace CKE {
	/**
	 * The Engine class serves as a default configuration for the engine. It initializes and manages
	 * various systems required for the engine to function. Users can create their own configurations
	 * by modifying the systems present in the engine.
	 */
	class CKE_API Engine
	{
	public:
		// Lifetime
		//-----------------------------------------------------------------------------

		// These functions must always be called in the following order:
		// engine.InitializeCore();
		// - (Configure Entity World) -
		// engine.InitializeEngine();
		// while (running) { engine.Update(); }
		// engine.Shutdown();

		// Initializes the lowest level systems and the resource system
		void InitializeCore();

		// Initializes the rest of the engine and builds the game world
		void InitializeEngine();

		// Performs a frame update in the engine
		void Update();

		// Shutdowns all of the engine systems and releases its resources
		void Shutdown();

		// System Accessors
		//-----------------------------------------------------------------------------

		RenderingSystem* GetRenderingSystem() { return &m_RenderingSystem; }
		EntitySystem*    GetEntitySystem() { return &m_EntitySystem; }
		ResourceSystem*  GetResourceSystem() { return &m_ResourceSystem; }
		InputSystem*     GetInputSystem() { return &m_InputSystem; }

	private:
		SystemsRegistry m_SystemsRegistry{};
		TaskSystem      m_TaskSystem{};
		EngineTime      m_EngineTime{};

		// Engine Systems
		ResourceSystem  m_ResourceSystem{};
		InputSystem     m_InputSystem{};
		RenderingSystem m_RenderingSystem{};
		EntitySystem    m_EntitySystem{};

		// Resource Loaders
		TextureLoader  m_TextureLoader{};
		PipelineLoader m_PipelineLoader{};
		MeshLoader     m_MeshLoader{};
		MaterialLoader m_MaterialLoader{};
		CubeMapLoader  m_CubeMapLoader{};
	};
} // namespace CKE
