cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Runtime_Core_Platform
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	Containers
	"${PUBLIC_MODULES}"
)

CK_Core_Module_Tests(
	Containers
)