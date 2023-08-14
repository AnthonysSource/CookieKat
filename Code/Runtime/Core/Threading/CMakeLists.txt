cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Runtime_Core_Platform
	CookieKat_Runtime_Core_Containers
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	Threading
	"${PUBLIC_MODULES}"
)