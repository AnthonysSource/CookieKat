cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

find_package(Vulkan REQUIRED)

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_EngineSystem
	CookieKat_Runtime_Systems_RenderAPI
	Vulkan::Vulkan
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	FrameGraph
	"${PUBLIC_MODULES}"
)

CK_Systems_Module_Tests(
	FrameGraph
)