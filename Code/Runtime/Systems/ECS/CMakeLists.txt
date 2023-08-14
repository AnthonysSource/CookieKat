cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_TaskSystem
	CookieKat_Runtime_Systems_EngineSystem
	CookieKat_Runtime_Systems_Input
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	ECS
	"${PUBLIC_MODULES}"
)

CK_Systems_Module_Tests(
	ECS
)