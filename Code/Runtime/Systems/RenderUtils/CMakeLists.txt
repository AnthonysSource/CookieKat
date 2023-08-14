cmake_minimum_required(VERSION 3.23)

# ------------------------------------------------------------------------------

set(PUBLIC_MODULES
	CookieKat_Runtime_Systems_RenderAPI
	"spirv-reflect-static"
)

# ------------------------------------------------------------------------------

CK_Systems_Module(
	RenderUtils
	"${PUBLIC_MODULES}"
)

add_subdirectory("ThirdParty/SPIRVReflect")