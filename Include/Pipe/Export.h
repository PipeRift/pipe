
#ifndef PIPE_API_H
#define PIPE_API_H

#ifdef PIPE_STATIC_DEFINE
#	define PIPE_API
#	define PIPE_NO_EXPORT
#else
#	ifndef PIPE_API
#		ifdef Pipe_EXPORTS
/* We are building this library */
#			define PIPE_API __declspec(dllexport)
#		else
/* We are using this library */
#			define PIPE_API __declspec(dllimport)
#		endif
#	endif

#	ifndef PIPE_NO_EXPORT
#		define PIPE_NO_EXPORT
#	endif
#endif

#ifndef PIPE_DEPRECATED
#	define PIPE_DEPRECATED __attribute__((__deprecated__))
#endif

#ifndef PIPE_DEPRECATED_EXPORT
#	define PIPE_DEPRECATED_EXPORT PIPE_API PIPE_DEPRECATED
#endif

#ifndef PIPE_DEPRECATED_NO_EXPORT
#	define PIPE_DEPRECATED_NO_EXPORT PIPE_NO_EXPORT PIPE_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#	ifndef PIPE_NO_DEPRECATED
#		define PIPE_NO_DEPRECATED
#	endif
#endif

#endif /* PIPE_API_H */
