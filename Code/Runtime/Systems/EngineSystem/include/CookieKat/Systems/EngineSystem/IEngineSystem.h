#pragma once

#include "CookieKat/Core/Platform/PlatformTime.h"

namespace CKE {
	// ID That uniquely identifies an engine system
	using EngineSystemID = u64;

	// Base interface for all engine systems
	// All of the engine systems can be accessed through the engine systems registry
	// Example: RenderingSystem, EntitiesSystem, ResourcesSystem...
	class IEngineSystem { };
}
