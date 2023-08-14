#pragma once

#include <optick.h>

// Use at the start of the application loop in the main thread
#define CKE_PROFILE_FRAME(X) OPTICK_FRAME(X)

#define CKE_PROFILE_EVENT(...) OPTICK_EVENT(__VA_ARGS__)
