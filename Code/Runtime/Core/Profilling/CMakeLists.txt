cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	OptickCore
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	Profilling
	"${PUBLIC_MODULES}"
)