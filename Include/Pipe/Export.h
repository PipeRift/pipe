
#pragma once

#ifndef PIPE_API
	#if defined(_WIN32)
		#if defined(PIPE_EXPORTS) && PIPE_EXPORTS
			#define PIPE_API __declspec(dllexport)
		#elif defined(PIPE_IMPORTS) && PIPE_IMPORTS
			#define PIPE_API __declspec(dllimport)
		#else
			#define PIPE_API
		#endif
	#elif __has_attribute(visibility)
		#define PIPE_API __attribute__((visibility("default")))
	#else
		#define PIPE_API
	#endif
#endif

#ifndef P_DEPRECATED
	#define P_DEPRECATED __declspec(deprecated)
#endif

#ifndef P_DEPRECATED_EXPORT
	#define P_DEPRECATED_EXPORT PIPE_API P_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
	#ifndef P_NO_DEPRECATED
		#define P_NO_DEPRECATED
	#endif
#endif
