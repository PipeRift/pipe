
#ifndef CORE_API_H
#define CORE_API_H

#ifdef CORE_STATIC_DEFINE
#  define CORE_API
#  define CORE_NO_EXPORT
#else
#  ifndef CORE_API
#    ifdef core_EXPORTS
        /* We are building this library */
#      define CORE_API __declspec(dllexport)
#    else
        /* We are using this library */
#      define CORE_API __declspec(dllimport)
#    endif
#  endif

#  ifndef CORE_NO_EXPORT
#    define CORE_NO_EXPORT 
#  endif
#endif

#ifndef CORE_DEPRECATED
#  define CORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef CORE_DEPRECATED_EXPORT
#  define CORE_DEPRECATED_EXPORT CORE_API CORE_DEPRECATED
#endif

#ifndef CORE_DEPRECATED_NO_EXPORT
#  define CORE_DEPRECATED_NO_EXPORT CORE_NO_EXPORT CORE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef CORE_NO_DEPRECATED
#    define CORE_NO_DEPRECATED
#  endif
#endif

#endif /* CORE_API_H */
