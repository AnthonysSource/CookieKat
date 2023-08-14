cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_EngineSystem
	CookieKat_Runtime_Systems_TaskSystem
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	Resources
	"${PUBLIC_MODULES}"
)