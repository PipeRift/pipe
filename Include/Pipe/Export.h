
#pragma once

#if defined(_WIN64) || defined(_WIN32)    // Windows
	#ifndef DLLEXPORT
		#define DLLEXPORT __declspec(dllexport)
	#endif
	#ifndef DLLIMPORT
		#define DLLIMPORT __declspec(dllimport)
	#endif
#else
	#ifndef DLLEXPORT
		#define DLLEXPORT
	#endif
	#ifndef DLLIMPORT
		#define DLLIMPORT
	#endif
#endif


#ifndef PIPE_API
	#ifdef PIPE_STATIC_DEFINE
		#define PIPE_API
		#define PIPE_NO_EXPORT
	#else
		#ifdef PIPE_EXPORTS
		    /* We are building this library */
			#define PIPE_API DLLEXPORT
		#else
		    /* We are using this library */
			#define PIPE_API DLLIMPORT
		#endif
	#endif

	#ifndef PIPE_NO_EXPORT
		#define PIPE_NO_EXPORT
	#endif
#endif

#ifndef P_DEPRECATED
	#define P_DEPRECATED __declspec(deprecated)
#endif

#ifndef P_DEPRECATED_EXPORT
	#define P_DEPRECATED_EXPORT PIPE_API P_DEPRECATED
#endif

#ifndef P_DEPRECATED_NO_EXPORT
	#define P_DEPRECATED_NO_EXPORT PIPE_NO_EXPORT P_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
	#ifndef P_NO_DEPRECATED
		#define P_NO_DEPRECATED
	#endif
#endif
