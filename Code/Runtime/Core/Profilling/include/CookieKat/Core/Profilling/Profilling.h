#pragma once

#include <optick.h>

// Use at the start of the application loop in the main thread
#define CKE_PROFILE_FRAME(X) OPTICK_FRAME(X)

#define CKE_PROFILE_EVENT(...) OPTICK_EVENT(__VA_ARGS__)
#define CKE_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)
#define CKE_PROFILE_CATEGORY(...) OPTICK_CATEGORY(__VA_ARGS__)
#define CKE_PROFILE_TAG(...) OPTICK_TAG(__VA_ARGS__)