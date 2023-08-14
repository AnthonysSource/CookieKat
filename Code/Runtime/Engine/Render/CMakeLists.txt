cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Core
	CookieKat_Systems
	CookieKat_Runtime_Engine_Entities
	CookieKat_Runtime_Engine_Resources
)

# ------------------------------------------------------------------------------

CK_Engine_Module(
	Render
	"${PUBLIC_MODULES}"
)