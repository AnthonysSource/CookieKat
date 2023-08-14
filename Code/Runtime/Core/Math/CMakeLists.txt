cmake_minimum_required(VERSION 3.23)

# Variables
# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	glm::glm
)

# ------------------------------------------------------------------------------

CK_Core_Module(
	"Math"
	"${PUBLIC_MODULES}"
)