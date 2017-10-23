#pragma once

#if !defined(P3_OS)
	#define P3_OS 1
	#define P3_OS_WINDOWS 0
	#define P3_OS_ANDROID 0
	#define P3_OS_LINUX 0
	#define P3_OS_MAC 0

	#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
		#undef P3_OS_WINDOWS
		#define P3_OS_WINDOWS 1
	#elif defined(__ANDROID__)
		#undef P3_OS_ANDROID
		#define P3_OS_ANDROID 1
	#elif defined(__linux__) || defined(__unix__)
		#undef P3_OS_LINUX
		#define P3_OS_LINUX 1
	#elif defined(__APPLE__)
		#undef P3_OS_MAC
		#define P3_OS_MAC 1
	#endif
#endif