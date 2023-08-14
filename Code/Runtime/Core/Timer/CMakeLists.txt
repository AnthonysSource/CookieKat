cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Runtime_Core_Platform
	CookieKat_Runtime_Core_Math
	CookieKat_Runtime_Core_Profilling
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	Time
	"${PUBLIC_MODULES}"
)


CK_Core_Module_Tests(
	Time
)