cmake_minimum_required(VERSION 3.23)


# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Runtime_Core_Containers
	CookieKat_Runtime_Core_Platform
	CookieKat_Runtime_Core_FileSystem
	CookieKat_Runtime_Core_Memory
	RapidJSON
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	Serialization
	"${PUBLIC_MODULES}"
)

CK_Core_Module_Tests(
	Serialization
)