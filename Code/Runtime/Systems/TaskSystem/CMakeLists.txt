cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_EngineSystem
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	"TaskSystem"
	"${PUBLIC_MODULES}"
)

CK_Systems_Module_Tests(
	"TaskSystem"
)