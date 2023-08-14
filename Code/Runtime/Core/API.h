#ifdef CKE_BUILDING_DLL
#ifdef CKE_BUILD_IMPORT_LIB
#define CKE_API __declspec(dllexport)
#else
		#define CKE_API __declspec(dllimport)
#endif
#else
	#define CKE_API
#endif // CKE_BUILDING_DLL
