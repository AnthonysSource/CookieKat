cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Runtime_Systems_EngineSystem
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	"Input"
	"${PUBLIC_MODULES}"
)

CK_Systems_Module_Tests(
	"Input"
)