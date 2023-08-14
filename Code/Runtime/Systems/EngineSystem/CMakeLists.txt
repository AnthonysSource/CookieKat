cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	EngineSystem
	"${PUBLIC_MODULES}"
)