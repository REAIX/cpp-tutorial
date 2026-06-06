#ifndef CU_EXPORT_H
#define CU_EXPORT_H

#ifdef CU_BUILDING_DLL
    #ifdef _WIN32
        #define CU_API __declspec(dllexport)
    #else
        #define CU_API __attribute__((visibility("default")))
    #endif
#elif defined(CU_DLL)
    #ifdef _WIN32
        #define CU_API __declspec(dllimport)
    #else
        #define CU_API __attribute__((visibility("default")))
    #endif
#else
    #define CU_API
#endif

#endif
