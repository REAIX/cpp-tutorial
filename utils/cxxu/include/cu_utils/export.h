#ifndef CXXU_EXPORT_H
#define CXXU_EXPORT_H

#ifdef CXXU_BUILDING_DLL
    #ifdef _WIN32
        #define CXXU_API __declspec(dllexport)
    #else
        #define CXXU_API __attribute__((visibility("default")))
    #endif
#elif defined(CXXU_DLL)
    #ifdef _WIN32
        #define CXXU_API __declspec(dllimport)
    #else
        #define CXXU_API __attribute__((visibility("default")))
    #endif
#else
    #define CXXU_API
#endif

#endif
