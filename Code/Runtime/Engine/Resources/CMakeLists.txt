cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Systems
)

# ------------------------------------------------------------------------------

CK_Engine_Module(
	Resources
	"${PUBLIC_MODULES}"
)

CK_Engine_Module_Tests(
	Resources
)