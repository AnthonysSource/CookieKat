cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

find_package(Vulkan REQUIRED)

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_EngineSystem
	Vulkan::Vulkan
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	RenderAPI
	"${PUBLIC_MODULES}"
)

CK_Systems_Module_Tests(
	RenderAPI
)