
#ifndef SQLDBLIB_EXPORT_RAW_H
#define SQLDBLIB_EXPORT_RAW_H

#ifdef SQLDBLIB_STATIC_DEFINE
#  define SQLDBLIB_EXPORT_RAW
#  define SQLDBLIB_NO_EXPORT
#else
#  ifndef SQLDBLIB_EXPORT_RAW
#    ifdef SqlDbLib_EXPORTS
        /* We are building this library */
#      define SQLDBLIB_EXPORT_RAW __declspec(dllexport)
#    else
        /* We are using this library */
#      define SQLDBLIB_EXPORT_RAW __declspec(dllimport)
#    endif
#  endif

#  ifndef SQLDBLIB_NO_EXPORT
#    define SQLDBLIB_NO_EXPORT 
#  endif
#endif

#ifndef SQLDBLIB_DEPRECATED
#  define SQLDBLIB_DEPRECATED __declspec(deprecated)
#endif

#ifndef SQLDBLIB_DEPRECATED_EXPORT
#  define SQLDBLIB_DEPRECATED_EXPORT SQLDBLIB_EXPORT_RAW SQLDBLIB_DEPRECATED
#endif

#ifndef SQLDBLIB_DEPRECATED_NO_EXPORT
#  define SQLDBLIB_DEPRECATED_NO_EXPORT SQLDBLIB_NO_EXPORT SQLDBLIB_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef SQLDBLIB_NO_DEPRECATED
#    define SQLDBLIB_NO_DEPRECATED
#  endif
#endif
#define SQLDBLIB_EXPORT extern "C" SQLDBLIB_EXPORT_RAW
#endif /* SQLDBLIB_EXPORT_RAW_H */
